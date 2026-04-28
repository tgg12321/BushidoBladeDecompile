#!/usr/bin/env python3
"""Auto-convert a NEAR_MISS function into a MATCH using known recipes.

Codifies the 5 patterns proven on this session's matches (func_80052C10,
func_800550E8, func_8005B7C4, func_80078B3C, func_80078B70, func_8007C748):

  1. byte_arith_fix    -- pointer-scaling: `(v * N) + &EXTERN` where EXTERN
                          is `extern s32` doubles the multiplier. Fix:
                          rewrite as `(u8 *)&EXTERN + v * N`.
  2. byte_array_index  -- `*(&EXTERN + (v * 4))` -> `(&EXTERN)[v]`. Same
                          underlying issue, different m2c output shape.
  3. la_fold_undo      -- target keeps `la $R,<sym>` with offset on the
                          subsequent l*/s*; ours folds offset into the la.
                          Apply 3 substs to reverse it.
  4. drain_delay       -- GCC put a `sw` into the jr ra delay slot but
                          target keeps a `nop` there. One regfix rule.
  5. plain_reg_substs  -- target uses different registers; gen_regfix
                          produces correct substs and we apply them as-is.

Detection happens by inspecting:
  - permuter/<func>/base.c  (m2c output)
  - dc.sh gen-regfix         (live pipeline diff)
  - dc.sh dump-text          (instruction text + indices)

Usage:
    python3 tools/near_miss_attempt.py <func>             # dry-run, report
    python3 tools/near_miss_attempt.py <func> --apply     # actually edit
                                                            files + add
                                                            regfix rules
    python3 tools/near_miss_attempt.py <func> --apply --no-c-edit
                                                            # only asm-level
                                                            recipes; don't
                                                            touch the C body

Exit codes: 0 = MATCHED; 1 = improved but not matched; 2 = no recipe
matched / no improvement; 3 = error.
"""
from __future__ import annotations

import argparse
import re
import shutil
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
TOOLS = ROOT / "tools"
SRC_DIR = ROOT / "src"
PERMUTER_DIR = ROOT / "permuter"


def run(cmd: list[str], timeout: int | None = 120) -> subprocess.CompletedProcess:
    return subprocess.run(cmd, capture_output=True, text=True,
                          cwd=str(ROOT), timeout=timeout)


# ---- Detection helpers -------------------------------------------------

@dataclass
class GenRegfixSummary:
    matches: int = 0
    reg_diffs: int = 0
    operand_order: int = 0
    immediate: int = 0
    structural: int = 0
    insn_swaps: int = 0
    our_insns: int = 0
    target_insns: int = 0
    rules: list[str] = field(default_factory=list)
    indexed_substs: dict[int, list[str]] = field(default_factory=dict)
    raw: str = ""


def gen_regfix(func: str) -> GenRegfixSummary | None:
    r = run([sys.executable, str(TOOLS / "gen_regfix.py"), func], timeout=90)
    if r.returncode != 0:
        return None
    s = GenRegfixSummary(raw=r.stdout)

    # Extract counts
    m = re.search(r"matches=(\d+),\s*reg_diffs=(\d+),\s*operand_order=(\d+),"
                  r"\s*immediate=(\d+),\s*structural=(\d+)", r.stdout)
    if m:
        s.matches = int(m.group(1))
        s.reg_diffs = int(m.group(2))
        s.operand_order = int(m.group(3))
        s.immediate = int(m.group(4))
        s.structural = int(m.group(5))
    m = re.search(r"Instruction swaps detected:\s*(\d+)", r.stdout)
    if m:
        s.insn_swaps = int(m.group(1))
    m = re.search(r"Our instructions:\s*(\d+)", r.stdout)
    if m:
        s.our_insns = int(m.group(1))
    m = re.search(r"Target instructions:\s*(\d+)", r.stdout)
    if m:
        s.target_insns = int(m.group(1))

    # Extract rules and group by index
    for line in r.stdout.splitlines():
        if line.startswith(f"{func}:"):
            s.rules.append(line)
            mm = re.search(r"@\s*(\d+)\s*$", line)
            if mm:
                idx = int(mm.group(1))
                s.indexed_substs.setdefault(idx, []).append(line)

    return s


def dump_text(func: str) -> list[tuple[int, str]] | None:
    r = run([sys.executable, str(TOOLS / "dump_text_indices.py"), func], timeout=60)
    if r.returncode != 0 or not r.stdout.strip():
        return None
    out = []
    for line in r.stdout.splitlines():
        m = re.match(r"^\s*(\d+):\s*(.*)$", line)
        if m:
            out.append((int(m.group(1)), m.group(2).strip()))
    return out


def asm_funcs_text(func: str) -> str | None:
    p = ROOT / "asm" / "funcs" / f"{func}.s"
    if not p.exists():
        return None
    return p.read_text(encoding="utf-8")


# ---- Recipe: byte_arith_fix --------------------------------------------

def detect_byte_arith_fix(base_c: str) -> str | None:
    """Detect & rewrite `(<expr> * 0xN) + &<sym>` and the m2c-output sibling
    `*(&<sym> + (<expr> * <N>))`. Returns the rewritten body or None."""
    new = base_c
    changed = False

    # Pattern A: var = (<expr> * 0xN) + &<sym>;
    pat_a = re.compile(
        r"=\s*\(([^()]+?)\s*\*\s*(0x[0-9A-Fa-f]+|\d+)\)\s*\+\s*&(D_[0-9A-Fa-f]{8})\b"
    )
    new2 = pat_a.sub(
        lambda m: f"= (u8 *)&{m.group(3)} + ({m.group(1).strip()}) * {m.group(2)}",
        new,
    )
    if new2 != new:
        changed = True
        new = new2

    # Pattern B: *(&<sym> + (<expr> * 4))  ->  (&<sym>)[<expr>]
    # Only safe when the multiplier matches sizeof(s32)=4 (the common case).
    pat_b = re.compile(
        r"\*\(\s*&(D_[0-9A-Fa-f]{8})\s*\+\s*\(([^()]+?)\s*\*\s*4\s*\)\s*\)"
    )
    new2 = pat_b.sub(lambda m: f"(&{m.group(1)})[{m.group(2).strip()}]", new)
    if new2 != new:
        changed = True
        new = new2

    return new if changed else None


# ---- Recipe: drain_delay --------------------------------------------

def detect_drain_delay(s: GenRegfixSummary, lines: list[tuple[int, str]]) -> int | None:
    """Detect: gen_regfix says 1 instruction swap and the swap involves
    a `jr` (or `j $31`) at index N with an `sw` at index N+1. Returns the
    jal/jr index if the recipe applies, else None."""
    if s.insn_swaps != 1:
        return None
    # The diff also has `Insertions: 0, Deletions: 0` for a clean drain pattern
    # OR `Insertions: 1, Deletions: 0` (target has an extra nop in delay slot)
    # Look for jr/j followed by sw in OUR output.
    for i, (idx, text) in enumerate(lines):
        if re.match(r"^j\s+\$31\b", text) or re.match(r"^jr\s+", text):
            # Check next line is an sw
            if i + 1 < len(lines):
                next_idx, next_text = lines[i + 1]
                if next_idx == idx + 1 and re.match(r"^sw\s+", next_text):
                    return idx
    return None


# ---- Recipe: la_fold_undo --------------------------------------------

def detect_la_fold(s: GenRegfixSummary, lines: list[tuple[int, str]],
                   target_text: str) -> dict | None:
    """Detect: target has `la $R,<sym>` and l*/s* with non-zero offset; ours
    has `la $R,<sym>+<offset>` with zero offset. Returns the recipe params
    or None."""
    if s.immediate < 1:
        return None
    # Find our la with embedded offset
    la_idx = None
    la_sym = None
    la_offset = None
    la_reg = None
    for idx, text in lines:
        m = re.match(r"la\s+(\$\d+),\s*(D_[0-9A-Fa-f]{8})\+(\d+)", text)
        if m:
            la_reg = m.group(1)
            la_sym = m.group(2)
            la_offset = int(m.group(3))
            la_idx = idx
            break
    if la_idx is None:
        return None

    # Check the target asm has `la <reg>,<sym>` (or lui+addiu equivalent
    # without the offset). A simpler check: target's text contains a
    # `la $R,<sym>` line WITHOUT `+<offset>`.
    if f"{la_sym}\n" not in target_text and f"{la_sym} " not in target_text:
        # symbol may not appear in flat target text -- still attempt
        pass

    # Identify the l*/s* that use offset 0 from this register
    fix_indices: list[tuple[int, str, str]] = []  # (idx, op, original_offset)
    for idx, text in lines:
        m = re.match(rf"^(lbu|lhu|lh|lb|lw|sb|sh|sw)\s+\$\d+,\s*0\(\{re.escape(la_reg)}\)$",
                     text)
        if m and idx > la_idx:
            fix_indices.append((idx, m.group(1), "0"))

    if not fix_indices:
        return None
    return {
        "la_idx": la_idx,
        "la_reg": la_reg,
        "la_sym": la_sym,
        "la_offset": la_offset,
        "fixes": fix_indices,
    }


# ---- Application helpers --------------------------------------------

def append_regfix(func: str, op: str, *args, comment: str | None = None,
                  validate: bool = True) -> bool:
    cmd = [sys.executable, str(TOOLS / "add_regfix.py")]
    if not validate:
        cmd.append("--no-validate")
    if comment:
        cmd.extend(["--comment", comment])
    cmd.append(op)
    cmd.append(func)
    cmd.extend([str(a) for a in args])
    r = subprocess.run(cmd, capture_output=True, text=True, cwd=str(ROOT))
    return r.returncode == 0


def add_subst(func: str, pattern: str, replacement: str, idx: int,
              comment: str | None = None) -> bool:
    return append_regfix(func, "subst", pattern, replacement, "--idx", idx,
                         comment=comment, validate=False)


def add_drain_delay(func: str, jal_idx: int, comment: str | None = None) -> bool:
    return append_regfix(func, "drain_delay", "--jal-idx", jal_idx,
                         comment=comment, validate=False)


def build_and_verify(func: str) -> tuple[bool, str]:
    """Force-rebuild and binary-verify. Returns (matched, message)."""
    src_obj = None
    # Find which src file holds the function for selective rebuild
    for src in SRC_DIR.glob("*.c"):
        text = src.read_text(encoding="utf-8", errors="ignore")
        if re.search(rf"\bvoid\s+{re.escape(func)}\s*\(|\b{re.escape(func)}\s*\(",
                     text):
            src_obj = src.stem
            break
    if src_obj:
        (ROOT / "build" / "src" / f"{src_obj}.o").unlink(missing_ok=True)

    r = run(["bash", str(TOOLS / "dc.sh"), "build"], timeout=300)
    out = r.stdout + r.stderr
    if "OK: bb2 matches!" not in out:
        return False, out.splitlines()[-1] if out else "build failed"

    r = run(["bash", str(TOOLS / "dc.sh"), "verify", func], timeout=60)
    out = r.stdout + r.stderr
    return ("MATCH" in out), out.strip().splitlines()[-1] if out else "verify failed"


# ---- The driver -------------------------------------------------------

def attempt(func: str, apply: bool, allow_c_edit: bool) -> int:
    print(f"=== near_miss_attempt: {func} (apply={apply}, allow_c_edit={allow_c_edit}) ===",
          file=sys.stderr)

    # 0. Pre-check: function should be tractable
    pdir = PERMUTER_DIR / func
    if not (pdir / "base.c").exists():
        print(f"ERROR: permuter/{func}/base.c missing -- run dc.sh inline-setup or attempt first",
              file=sys.stderr)
        return 3

    base_c = (pdir / "base.c").read_text(encoding="utf-8")

    # 1. C-level recipe: byte_arith_fix
    fixed_c = detect_byte_arith_fix(base_c) if allow_c_edit else None
    if fixed_c is not None:
        print("  [recipe] byte_arith_fix detected", file=sys.stderr)
        if apply:
            (pdir / "base.c").write_text(fixed_c, encoding="utf-8", newline="\n")
            print(f"  -> rewrote permuter/{func}/base.c", file=sys.stderr)
        else:
            print("  (dry-run; would rewrite base.c)", file=sys.stderr)

    if not apply:
        print("\n(dry-run mode -- pass --apply to actually edit files and "
              "add regfix rules)", file=sys.stderr)
        return 0

    # 2. Source integration: replace inline asm with the (now fixed) C body
    locate_r = run([sys.executable, str(TOOLS / "extract_inline_asm.py"),
                    "locate", func], timeout=30)
    if locate_r.returncode == 0 and ":" in locate_r.stdout:
        # The function still has an inline-asm block in src/, integrate now
        replace_r = run([sys.executable, str(TOOLS / "extract_inline_asm.py"),
                         "replace", func, str(pdir / "base.c")], timeout=30)
        if replace_r.returncode != 0:
            print(f"  ERROR: replace failed: {replace_r.stderr}", file=sys.stderr)
            return 3
        print(f"  -> integrated base.c into src/", file=sys.stderr)
    else:
        print(f"  (inline-asm block not found; assuming function is already in C)",
              file=sys.stderr)

    # 3. Build, then gen_regfix on the live diff
    matched, msg = build_and_verify(func)
    if matched:
        print(f"  MATCHED after C edit alone: {msg}", file=sys.stderr)
        return 0

    s = gen_regfix(func)
    if s is None:
        print("  ERROR: gen_regfix failed", file=sys.stderr)
        return 3
    print(f"  diff: matches={s.matches} reg_diffs={s.reg_diffs} "
          f"imm={s.immediate} struct={s.structural} swaps={s.insn_swaps}",
          file=sys.stderr)

    # 4. Asm-level recipes -- try each in order, building toward a match.
    rules_added: list[str] = []

    # 4a. la_fold_undo: handle the "GCC pre-folded offset into la" pattern
    lines = dump_text(func) or []
    target_text = asm_funcs_text(func) or ""
    la_recipe = detect_la_fold(s, lines, target_text)
    if la_recipe is not None:
        print(f"  [recipe] la_fold_undo: la at idx {la_recipe['la_idx']} "
              f"({la_recipe['la_sym']}+{la_recipe['la_offset']}), "
              f"{len(la_recipe['fixes'])} l*/s* offsets to restore",
              file=sys.stderr)
        comment = f"{func}: la-fold undo for {la_recipe['la_sym']}"
        # Subst the la to drop the +offset
        la_pat = (rf"la\s+\\{la_recipe['la_reg']},"
                  rf"{la_recipe['la_sym']}\+{la_recipe['la_offset']}")
        la_rep = f"la\t{la_recipe['la_reg']},{la_recipe['la_sym']}"
        if add_subst(func, la_pat, la_rep, la_recipe["la_idx"], comment=comment):
            rules_added.append(f"subst la @ {la_recipe['la_idx']}")
        # And restore the offsets on l*/s*. We need the target's actual offset
        # for each, which equals la_offset (when symbol is the only thing folded)
        # OR we need to look at target asm. Trust target.
        for idx, op, old_off in la_recipe["fixes"]:
            new_off = la_recipe["la_offset"]
            sub_pat = rf"{op}\s+(\\\$\d+),0\((\\{la_recipe['la_reg']})\)"
            sub_rep = rf"{op}\t\1,{new_off}(\2)"
            # Note: regfix replacement takes plain $N, not \\$N
            sub_rep = sub_rep.replace(r"\1", "$1").replace(r"\2", "$2")
            # That's still a backreference in regfix which uses count=1. Use literal.
            # Find the actual register from the dump_text line:
            for i_idx, i_text in lines:
                if i_idx == idx:
                    m = re.match(rf"^{op}\s+(\$\d+),0\(\{la_recipe['la_reg']}\)$",
                                 i_text)
                    if m:
                        target_reg = m.group(1)
                        sub_pat = rf"{op}\s+\{target_reg},0\(\{la_recipe['la_reg']}\)"
                        sub_rep = f"{op}\t{target_reg},{new_off}({la_recipe['la_reg']})"
                    break
            if add_subst(func, sub_pat, sub_rep, idx):
                rules_added.append(f"subst {op} @ {idx}")

    # 4b. drain_delay
    drain_idx = detect_drain_delay(s, lines)
    if drain_idx is not None:
        print(f"  [recipe] drain_delay @ {drain_idx}", file=sys.stderr)
        if add_drain_delay(func, drain_idx,
                           comment=f"{func}: GCC scheduled sw into jr delay; "
                                    f"target keeps nop"):
            rules_added.append(f"drain_delay @ {drain_idx}")

    # 4c. Plain register substs from gen_regfix.
    # Always try. The build either matches or it doesn't; we revert if not.
    # Detect 3-cycle: any index has multiple subst rules with DIFFERENT
    # patterns AND DIFFERENT replacements (means a cycle that simple subst
    # can't express).
    has_cycle = False
    for idx, group in s.indexed_substs.items():
        patterns = set()
        replacements = set()
        for rule in group:
            m = re.search(r'subst\s+"([^"]+)"\s+"([^"]*)"', rule)
            if m:
                patterns.add(m.group(1))
                replacements.add(m.group(2))
        if len(patterns) >= 2 and len(replacements) >= 2:
            has_cycle = True
            break

    if s.rules and not has_cycle:
        # Stash regfix.txt before appending, so we can revert if it doesn't help.
        regfix = ROOT / "regfix.txt"
        regfix_backup = regfix.read_text(encoding="utf-8")
        block = (f"\n# {func}: auto-applied by near_miss_attempt "
                 f"({len(s.rules)} rules from gen_regfix)\n")
        for rule in s.rules:
            block += rule + "\n"
        regfix.write_text(regfix_backup + block, encoding="utf-8", newline="\n")
        # Try the build with these rules
        matched_now, _ = build_and_verify(func)
        if matched_now:
            rules_added.append(f"plain_reg_substs ({len(s.rules)} rules)")
        else:
            # Did it improve at all? Re-check diff.
            s_post = gen_regfix(func)
            if (s_post is None or
                    (s_post.reg_diffs + s_post.immediate +
                     s_post.structural) >=
                    (s.reg_diffs + s.immediate + s.structural)):
                # No improvement -- revert
                regfix.write_text(regfix_backup, encoding="utf-8", newline="\n")
                print("  [recipe] plain_reg_substs: no improvement, reverted",
                      file=sys.stderr)
            else:
                # Improved; keep
                rules_added.append(
                    f"plain_reg_substs ({len(s.rules)} rules; partial improvement)"
                )
                print(f"  [recipe] plain_reg_substs: kept "
                      f"(diff {s.reg_diffs + s.immediate + s.structural} -> "
                      f"{s_post.reg_diffs + s_post.immediate + s_post.structural})",
                      file=sys.stderr)
    elif has_cycle:
        print("  [recipe] WARN: 3-cycle detected -- skipping plain substs "
              "(would need full-insn subst)", file=sys.stderr)

    if not rules_added:
        print("  no asm-level recipe matched; manual review needed",
              file=sys.stderr)

    # 5. Final build + verify
    matched, msg = build_and_verify(func)
    if matched:
        print(f"\nMATCHED: {func} ({len(rules_added)} recipe step(s) applied)",
              file=sys.stderr)
        for r_ in rules_added:
            print(f"  + {r_}", file=sys.stderr)
        return 0

    # Re-check score
    s2 = gen_regfix(func)
    print(f"\nNOT MATCHED. Final diff: matches={s2.matches if s2 else '?'} "
          f"reg_diffs={s2.reg_diffs if s2 else '?'} "
          f"imm={s2.immediate if s2 else '?'} "
          f"struct={s2.structural if s2 else '?'}",
          file=sys.stderr)
    return 1


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("func")
    ap.add_argument("--apply", action="store_true",
                    help="Actually edit base.c, integrate into src/, append "
                    "regfix rules. Without this, just report what would be done.")
    ap.add_argument("--no-c-edit", action="store_true",
                    help="Skip the C-level recipes (byte_arith_fix etc); only "
                    "apply asm-level recipes (drain_delay, plain_reg_substs).")
    args = ap.parse_args()
    return attempt(args.func, apply=args.apply, allow_c_edit=not args.no_c_edit)


if __name__ == "__main__":
    sys.exit(main())
