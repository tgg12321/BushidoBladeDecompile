#!/usr/bin/env python3
"""Sequence-aligned binary diff: find STRUCTURAL divergences between mine and
target, hiding cascade caused by single-position length mismatches.

This complements `dc.sh diff` (which aligns by index, so a 1-instruction
length difference cascades to mark every subsequent line as "different").
This tool uses difflib's sequence matcher with relocation-masked binary
opcodes to align across length differences and report ONLY the real
structural changes plus suggestions for known recipes.

Use this FIRST when a function fails to match. It tells you in seconds
whether you're 1-instruction-off (delay-slot fill, label shift) vs
structurally wrong (different C body needed) — saving long detours
through register asm hints and permuter runs.

Usage:
    python3 tools/diff_align.py <func>
    bash tools/dc.sh diff-align <func>
"""
from __future__ import annotations

import argparse
import difflib
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

RESET = "\033[0m"
RED = "\033[31m"
GREEN = "\033[32m"
YELLOW = "\033[33m"
CYAN = "\033[36m"
BOLD = "\033[1m"
DIM = "\033[2m"


def read_target_words(func: str) -> tuple[list[int], list[str]]:
    """Read asm/funcs/<func>.s, return (binary_words, instruction_strings)."""
    path = ROOT / "asm" / "funcs" / f"{func}.s"
    if not path.exists():
        raise FileNotFoundError(str(path))
    words: list[int] = []
    insns: list[str] = []
    line_re = re.compile(r"\s+/\*\s+\S+\s+\S+\s+([0-9A-Fa-f]+)\s+\*/\s+(.*)")
    for line in path.read_text(encoding="utf-8").splitlines():
        m = line_re.match(line)
        if m and len(m.group(1)) == 8:
            hexbytes = m.group(1)
            # Little-endian byte order in target.s; reverse to big-endian word.
            be = hexbytes[6:8] + hexbytes[4:6] + hexbytes[2:4] + hexbytes[0:2]
            words.append(int(be, 16))
            insns.append(m.group(2).strip())
    return words, insns


def find_object_for_func(func: str) -> Path | None:
    """Find which build/src/<file>.o EXPORTS the function symbol.

    Uses `nm` to match by symbol DEFINITION (T = text section, defined),
    not by source-text scan (which mistakes call sites for definitions).
    """
    build_dir = ROOT / "build" / "src"
    if not build_dir.is_dir():
        return None
    for o in sorted(build_dir.glob("*.o")):
        try:
            out = subprocess.check_output(
                ["mipsel-linux-gnu-nm", str(o)], stderr=subprocess.DEVNULL
            ).decode()
        except subprocess.CalledProcessError:
            continue
        # nm output: `00000000 T func_NAME` for defined text symbols.
        # Match the definition flag (T or t for local) — never U (undefined).
        for line in out.splitlines():
            parts = line.strip().split()
            if len(parts) >= 3 and parts[1] in ("T", "t") and parts[2] == func:
                return o
    return None


def read_mine_words(func: str) -> tuple[list[int], list[str]]:
    """objdump build/src/<file>.o, extract function's binary words and asm."""
    o = find_object_for_func(func)
    if o is None:
        raise FileNotFoundError(f"no built .o file contains {func} (run make?)")
    out = subprocess.check_output(
        ["mipsel-linux-gnu-objdump", "-d", "-EL", str(o)]
    ).decode()
    in_func = False
    words: list[int] = []
    insns: list[str] = []
    for line in out.splitlines():
        if f"<{func}>:" in line:
            in_func = True
            continue
        if in_func and re.match(r"^[0-9a-f]+\s+<", line):
            break
        if not in_func:
            continue
        m = re.match(r"\s+[0-9a-f]+:\s+([0-9a-f]+)\s+(.*)", line)
        if m:
            words.append(int(m.group(1), 16))
            insns.append(m.group(2).strip())
    if not words:
        raise RuntimeError(f"empty disassembly for {func}")
    return words, insns


# MIPS load/store opcodes whose 16-bit immediate is often a %lo(symbol)
# relocation. Mine (unlinked) has 0 in those bits, target (linked) has the
# resolved value. Mask them for alignment so they don't show as cascading
# diffs.
_LS_OPCODES = {
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,  # lb, lh, lwl, lw, lbu, lhu, lwr
    0x28, 0x29, 0x2A, 0x2B, 0x2E,              # sb, sh, swl, sw, swr
}


def mask_for_alignment(word: int) -> int:
    """Mask out relocation-only bits so unlinked mine and linked target align.

    - lui (op 0x0F): low 16 bits are %hi(symbol).
    - jal (op 0x03), j (op 0x02): low 26 bits are absolute target.
    - lw/sw/lh/sh/lb/lbu/lhu/sb (loads/stores): low 16 is often %lo(symbol)
      OR a stack/struct offset. Mask aggressively — false negatives on
      stack-offset bugs are rare and `dc.sh verify` catches them anyway.
    - addiu/ori (op 0x09, 0x0D): low 16 is often %lo(symbol) when it's the
      addiu in a `lui+addiu` pair. Mask these too.
    """
    op = (word >> 26) & 0x3F
    if op == 0x0F:
        return word & 0xFFFF0000
    if op == 0x03 or op == 0x02:
        return word & 0xFC000000
    if op in _LS_OPCODES:
        return word & 0xFFFF0000
    if op == 0x09 or op == 0x0D:  # addiu, ori (often %lo of symbol)
        return word & 0xFFFF0000
    return word


# Branch opcodes (for delay-slot detection)
BRANCH_OPCODES = {
    0x04, 0x05, 0x06, 0x07,  # beq, bne, blez, bgtz
    0x01,  # bgez/bltz family (regimm)
    0x02, 0x03,              # j, jal
}


def is_branch(word: int) -> bool:
    op = (word >> 26) & 0x3F
    if op in BRANCH_OPCODES:
        return True
    # jr/jalr (special-r encoded)
    if op == 0 and (word & 0x3F) in (0x08, 0x09):
        return True
    return False


def is_nop(word: int) -> bool:
    return word == 0


def signed_branch_offset(word: int) -> int | None:
    """For beq/bne/blez/bgtz/bgez/bltz, return signed 16-bit offset (in
    instructions, not bytes). None for non-branches."""
    op = (word >> 26) & 0x3F
    if op in (0x04, 0x05, 0x06, 0x07, 0x01):
        off = word & 0xFFFF
        if off & 0x8000:
            off -= 0x10000
        return off
    return None


def detect_recipes(target_words, target_insns, mine_words, mine_insns,
                   opcodes) -> list[str]:
    """Heuristic detectors for known regfix recipes. Returns a list of
    suggestion strings.

    Each detector is conservative — it errs on the side of NOT firing rather
    than firing on coincidences, since wrong suggestions waste agent time.
    """
    suggestions: list[str] = []

    # Detector 1: delay-slot fill missing.
    # Mine has `nop` immediately after a branch/jal, target has a non-nop at
    # the same position (post-alignment). Recipe: insert_after the branch.
    for tag, ti, tj, mi, mj in opcodes:
        if tag != "replace":
            continue
        # Walk pairwise within the replace block.
        for k in range(min(tj - ti, mj - mi)):
            t_w = target_words[ti + k]
            m_w = mine_words[mi + k]
            if is_nop(m_w) and not is_nop(t_w):
                # Check the previous instruction: must be a branch/jal.
                if mi + k - 1 >= 0 and is_branch(mine_words[mi + k - 1]):
                    suggestions.append(
                        f"  [delay-fill] mine has nop in delay slot at mine "
                        f"idx {mi + k} after branch '{mine_insns[mi + k - 1]}';\n"
                        f"               target has '{target_insns[ti + k]}' there.\n"
                        f"               Recipe: insert_after \"{target_insns[ti + k].strip()}\" "
                        f"@ <maspsx idx of branch> (use dump_text_indices to find it)."
                    )

    # Detector 2: branch-target-off-by-1 (label shift).
    # Both mine and target have a branch at the SAME aligned position, with
    # the same opcode and registers, but offset differs by exactly ±1
    # instruction (= ±4 bytes). Cause: GCC's auto-label `.L<N>` is attached
    # to the wrong instruction position. Recipe: insert_label + subst.
    for tag, ti, tj, mi, mj in opcodes:
        if tag != "replace" or (tj - ti) != (mj - mi):
            continue
        for k in range(tj - ti):
            t_w = target_words[ti + k]
            m_w = mine_words[mi + k]
            t_off = signed_branch_offset(t_w)
            m_off = signed_branch_offset(m_w)
            if t_off is None or m_off is None:
                continue
            # Same opcode + rs + rt? Mask out the immediate (low 16) and
            # compare the high 16 bits.
            if (t_w & 0xFFFF0000) == (m_w & 0xFFFF0000) and abs(t_off - m_off) == 1:
                suggestions.append(
                    f"  [label-shift] branch at idx ~{mi + k} differs by 1 instruction "
                    f"in offset.\n"
                    f"               target: {target_insns[ti + k]}\n"
                    f"               mine:   {mine_insns[mi + k]}\n"
                    f"               Recipe: insert_label \".L_new:\" @ <branch_target_idx ± 1>; "
                    f"subst the branch's `.L<N>` to `.L_new`.\n"
                    f"               (mine's label attaches to the wrong instruction; shift it.)"
                )

    # Detector 3: mine has `la $X, sym` (lui+addiu) outside, target has
    # inline `lui+addiu+addu` at the access site. Hoist-removal recipe.
    # Heuristic: target has 2 EXTRA instructions of (lui $r, %hi; addiu $r,
    # $r, %lo) at a position where mine has none, and mine has a `la` (or
    # equivalent lui+addiu pair) somewhere earlier.
    # Cheap check: if mine has fewer instructions and there's a long-running
    # `delete` block in target around lui/addiu pairs, flag it.
    target_lui_addiu_pairs = 0
    mine_lui_addiu_pairs = 0
    for w in target_words:
        if (w >> 26) & 0x3F == 0x0F:
            target_lui_addiu_pairs += 1
    for w in mine_words:
        if (w >> 26) & 0x3F == 0x0F:
            mine_lui_addiu_pairs += 1
    if target_lui_addiu_pairs > mine_lui_addiu_pairs + 1:
        suggestions.append(
            f"  [hoist-removal?] target has {target_lui_addiu_pairs} lui instructions, "
            f"mine has {mine_lui_addiu_pairs} (target +{target_lui_addiu_pairs - mine_lui_addiu_pairs}).\n"
            f"               If mine hoists a `la $X, sym` out of a loop where target inlines "
            f"`lui+addiu+addu`, delete the la and add inline lui+addiu in body.\n"
            f"               Recipe: delete @<la_idx>; insert lui/addiu pair before the addu; "
            f"subst the addu source register."
        )

    return suggestions


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Sequence-aligned binary diff with structural divergence detection."
    )
    ap.add_argument("func")
    ap.add_argument("--no-color", action="store_true",
                    help="Suppress ANSI color codes")
    ap.add_argument("--max-show", type=int, default=10,
                    help="Max instructions per hunk to show (default: 10)")
    args = ap.parse_args()

    if args.no_color:
        global RESET, RED, GREEN, YELLOW, CYAN, BOLD, DIM
        RESET = RED = GREEN = YELLOW = CYAN = BOLD = DIM = ""

    func = args.func
    try:
        t_words, t_insns = read_target_words(func)
    except FileNotFoundError as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1
    try:
        m_words, m_insns = read_mine_words(func)
    except (FileNotFoundError, RuntimeError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    delta = len(m_words) - len(t_words)
    print(f"{BOLD}=== {func} ==={RESET}")
    print(f"  target: {len(t_words)} insns ({len(t_words) * 4} bytes)")
    print(f"  mine:   {len(m_words)} insns ({len(m_words) * 4} bytes)")
    if delta == 0:
        print(f"  Δ:      {GREEN}same length{RESET}")
    elif delta > 0:
        print(f"  Δ:      {YELLOW}mine has {delta} more instruction(s) than target{RESET}")
    else:
        print(f"  Δ:      {YELLOW}mine is {-delta} instruction(s) short{RESET}")
    print()

    # Sequence alignment with relocation masking.
    t_masked = [mask_for_alignment(w) for w in t_words]
    m_masked = [mask_for_alignment(w) for w in m_words]
    matcher = difflib.SequenceMatcher(None, t_masked, m_masked)
    opcodes = matcher.get_opcodes()

    real_changes = [op for op in opcodes if op[0] != "equal"]
    if not real_changes:
        # All differences (if any) are pure relocation. Dump bare counts.
        print(f"  {GREEN}No structural divergences.{RESET} (Mine and target are "
              f"binary-equivalent up to relocations.)")
        return 0

    print(f"{BOLD}--- structural divergences (relocation-masked) ---{RESET}")
    print(f"  Showing {len(real_changes)} change region(s). Cascading register-")
    print(f"  naming differences caused by length mismatches are HIDDEN.\n")

    for tag, ti, tj, mi, mj in real_changes:
        t_count = tj - ti
        m_count = mj - mi
        if tag == "insert":
            print(f"  {GREEN}[mine-only +{m_count}]{RESET} target_idx={ti} | "
                  f"mine_idx={mi}..{mj - 1}")
            for k in range(min(args.max_show, mj - mi)):
                print(f"    M[{mi + k:3d}] {mine_insns_str(m_insns, mi + k)}")
            if mj - mi > args.max_show:
                print(f"    ... ({mj - mi - args.max_show} more)")
        elif tag == "delete":
            print(f"  {RED}[target-only +{t_count}]{RESET} target_idx={ti}..{tj - 1} | "
                  f"mine_idx={mi}")
            for k in range(min(args.max_show, tj - ti)):
                print(f"    T[{ti + k:3d}] {mine_insns_str(t_insns, ti + k)}")
            if tj - ti > args.max_show:
                print(f"    ... ({tj - ti - args.max_show} more)")
        elif tag == "replace":
            print(f"  {YELLOW}[replace]{RESET} target_idx={ti}..{tj - 1} (n={t_count}) "
                  f"↔ mine_idx={mi}..{mj - 1} (n={m_count})")
            n = max(t_count, m_count)
            for k in range(min(args.max_show, n)):
                t = mine_insns_str(t_insns, ti + k) if k < t_count else "—"
                m = mine_insns_str(m_insns, mi + k) if k < m_count else "—"
                print(f"    T[{ti + k:3d}] {t:<48s} M[{mi + k:3d}] {m}")
            if n > args.max_show:
                print(f"    ... ({n - args.max_show} more)")
        print()

    # Recipe suggestions.
    suggestions = detect_recipes(t_words, t_insns, m_words, m_insns, opcodes)
    if suggestions:
        print(f"{BOLD}--- suggested recipes ---{RESET}")
        for s in suggestions:
            print(s)
            print()

    print(f"{DIM}(Use `dc.sh diff {func}` for full positional side-by-side; "
          f"this tool focuses on STRUCTURAL diffs only.){RESET}")
    return 0


def mine_insns_str(insns: list[str], idx: int) -> str:
    if 0 <= idx < len(insns):
        return insns[idx]
    return ""


if __name__ == "__main__":
    sys.exit(main())
