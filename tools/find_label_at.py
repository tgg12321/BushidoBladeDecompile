#!/usr/bin/env python3
"""Resolve a target byte address to the right `.L<N>` label in mine's compile.

Usage:
    python3 tools/find_label_at.py <func_name> <hex_address>
    python3 tools/find_label_at.py func_8007352C 0x800736DC

Why this exists: when a regfix rule has a hardcoded `.L<N>` replacement and
file-wide GCC label numbering shifts (because some other function in the same
.c grew or shrank), the rule still applies but the label name now resolves to
a different address. The rule's branch encodes a wrong target.

Manual debugging this is painful — you try `.L905` → `.L926` → `.L922` →
`insert_label @ 103`/`104`/`105` empirically, each costing a build cycle. This
tool computes the answer in one shot:

  1. Disassembles the .o file containing <func_name>.
  2. Maps each binary offset to a maspsx text idx (via dump-text).
  3. Finds the `.L<N>` whose resolved address == target address. Reports it.
  4. If no exact match, suggests `insert_label @ <text_idx>` for the right
     position (the text idx that maps to the target binary offset).

Output:
  MATCH: .L922 (text idx 105, binary idx 109)
or
  SUGGEST: insert_label '.L_F352C_t:' @ 105
           (this puts a label POINTING TO text idx 105 = binary idx 108)
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent


def find_o_file_for_func(func: str) -> Path | None:
    """Locate the .o that defines <func>."""
    build_src = ROOT / "build" / "src"
    if not build_src.exists():
        return None
    for o in sorted(build_src.glob("*.o")):
        try:
            r = subprocess.run(
                ["mipsel-linux-gnu-objdump", "-t", str(o)],
                capture_output=True, text=True, timeout=30,
            )
        except FileNotFoundError:
            return None
        # Look for symbol matching func name
        for line in r.stdout.splitlines():
            if re.search(rf"\b{re.escape(func)}\b", line) and " F " in line:
                return o
    return None


def get_func_link_address(func: str) -> int | None:
    """Read named_syms.txt or undefined_syms_auto.txt to find linked address."""
    for fname in ("named_syms.txt", "undefined_syms_auto.txt"):
        p = ROOT / fname
        if not p.exists():
            continue
        for line in p.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            # format: <name> = 0xADDR;
            m = re.match(rf"^{re.escape(func)}\s*=\s*(0x[0-9A-Fa-f]+)", line)
            if m:
                return int(m.group(1), 16)
    # Fallback: parse linker map
    map_path = ROOT / "build" / "bb2.map"
    if map_path.exists():
        for line in map_path.read_text(encoding="utf-8", errors="ignore").splitlines():
            if func in line:
                m = re.search(r"\b(0x80[0-9a-fA-F]{6})\b", line)
                if m:
                    return int(m.group(1), 16)
    return None


def get_obj_func_offset(o_path: Path, func: str) -> int | None:
    """Find <func>'s offset within the .o file."""
    r = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-t", str(o_path)],
        capture_output=True, text=True, timeout=30,
    )
    for line in r.stdout.splitlines():
        # symbol table: ADDR FLAGS SECTION SIZE NAME
        if " F " not in line:
            continue
        parts = line.split()
        if not parts:
            continue
        if parts[-1] == func:
            try:
                return int(parts[0], 16)
            except ValueError:
                continue
    return None


def disassemble_func(o_path: Path, func: str) -> tuple[int, list[tuple[int, str]]]:
    """Return (func_offset_in_o, list of (binary_offset, asm_line)) for func."""
    func_off = get_obj_func_offset(o_path, func)
    r = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-d", str(o_path)],
        capture_output=True, text=True, timeout=30,
    )
    in_func = False
    out = []
    for line in r.stdout.splitlines():
        if re.match(rf"^[0-9a-f]+\s+<{re.escape(func)}>:", line):
            in_func = True
            continue
        if not in_func:
            continue
        # Next function header ends our scan
        if re.match(r"^[0-9a-f]+\s+<\w+>:", line):
            break
        m = re.match(r"^\s*([0-9a-f]+):\s+([0-9a-f]+)\s+(.+)$", line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        out.append((addr, m.group(3).strip()))
    return func_off or 0, out


def parse_dump_text(func: str) -> tuple[list[tuple[int, str, bool]], list[tuple[int, str]]]:
    """Run dc.sh dump-text and return:
        instructions: list of (text_idx, text, is_pseudo)
        labels:       list of (text_idx_AFTER, label_name)  # label points to next inst
    """
    r = subprocess.run(
        ["bash", "tools/dc.sh", "dump-text", func],
        capture_output=True, text=True, timeout=120, cwd=str(ROOT),
    )
    insts: list[tuple[int, str, bool]] = []
    labels: list[tuple[int, str]] = []
    pending_labels: list[str] = []
    for raw in r.stdout.splitlines():
        # instruction line: "  N: text"
        m_inst = re.match(r"^\s+(\d+):\s+(.*)$", raw)
        if m_inst:
            idx = int(m_inst.group(1))
            text = m_inst.group(2).rstrip()
            is_pseudo = _is_pseudo(text)
            insts.append((idx, text, is_pseudo))
            for lbl in pending_labels:
                labels.append((idx, lbl))
            pending_labels.clear()
            continue
        # label line: "    : .Lname:" (also matches " : <label>:")
        m_lbl = re.match(r"^\s+:\s+(\.L\w+):\s*$", raw)
        if m_lbl:
            pending_labels.append(m_lbl.group(1))
            continue
    return insts, labels


_PSEUDO_PATTERNS = [
    # `lw $X, sym` (no offset paren) -- non-gp-rel pseudo, expands to lui+lw
    re.compile(r"^lw\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*$"),
    re.compile(r"^lw\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*#"),
    # `sw $X, sym` -- pseudo
    re.compile(r"^sw\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*$"),
    re.compile(r"^sw\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*#"),
    # `lh / lhu / sh / lb / lbu / sb` against bare sym -- pseudo
    re.compile(r"^(lhu?|sh|lbu?|sb)\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*$"),
    re.compile(r"^(lhu?|sh|lbu?|sb)\s+\$\w+\s*,\s*[A-Za-z_]\w*\s*#"),
    # `la $X, sym` -- always 2 inst (lui+addiu)
    re.compile(r"^la\s+\$\w+\s*,"),
]


def _is_pseudo(line: str) -> bool:
    """Return True if this maspsx line expands to 2+ binary instructions."""
    txt = re.split(r"\s*#", line, maxsplit=1)[0].strip()
    for p in _PSEUDO_PATTERNS:
        if p.match(txt):
            return True
    return False


def parse_regfix_inserts_deletes(func: str) -> tuple[list[int], list[int], list[int]]:
    """Read regfix.txt + regfix_stage2.txt for func.

    Returns (inserts, insert_afters, deletes) where each list contains the idx
    values for that op:
      - insert @ N        adds 1 binary inst BEFORE idx N. Shifts idx >= N by +1.
      - insert_after @ N  adds 1 binary inst AFTER idx N. Shifts idx > N by +1.
      - delete @ N        removes 1 binary inst at idx N. Shifts idx > N by -1.
    """
    inserts: list[int] = []
    insert_afters: list[int] = []
    deletes: list[int] = []
    for fname in ("regfix.txt", "regfix_stage2.txt"):
        p = ROOT / fname
        if not p.exists():
            continue
        for raw in p.read_text(encoding="utf-8").splitlines():
            line = raw.strip()
            if not line.startswith(func + ":"):
                continue
            # `<func>: insert "..." @ N` (NOT insert_after, NOT insert_label)
            m_ins = re.match(rf'^{re.escape(func)}\s*:\s*insert\s+"[^"]*"\s*@\s*(\d+)\s*$', line)
            if m_ins:
                inserts.append(int(m_ins.group(1)))
                continue
            m_iaft = re.match(rf'^{re.escape(func)}\s*:\s*insert_after\s+"[^"]*"\s*@\s*(\d+)\s*$', line)
            if m_iaft:
                insert_afters.append(int(m_iaft.group(1)))
                continue
            m_del = re.match(rf'^{re.escape(func)}\s*:\s*delete\s*@\s*(\d+)\s*$', line)
            if m_del:
                deletes.append(int(m_del.group(1)))
                continue
    return inserts, insert_afters, deletes


def text_idx_to_binary_idx(insts: list[tuple[int, str, bool]], target_text_idx: int,
                          inserts: list[int] = None,
                          insert_afters: list[int] = None,
                          deletes: list[int] = None) -> int:
    """Compute the binary instruction index of the line at text idx target_text_idx.

    Accounts for:
      - pseudo expansions (1 text → 2 binary)
      - insert @ N (shifts idx >= N by +1 binary)
      - insert_after @ N (shifts idx > N by +1 binary)
      - delete @ N (removes idx N, shifts idx > N by -1)
    """
    inserts = inserts or []
    insert_afters = insert_afters or []
    deletes = deletes or []

    # Compute base binary position by walking the (post-delete) instruction list.
    binary = 0
    for idx, _text, is_pseudo in insts:
        if idx == target_text_idx:
            break
        if idx in deletes:
            continue
        binary += 2 if is_pseudo else 1

    # Apply insert deltas. `insert @ N` shifts everything at or after N.
    for ins in inserts:
        if ins <= target_text_idx:
            binary += 1
    # `insert_after @ N` shifts everything strictly after N.
    for ia in insert_afters:
        if ia < target_text_idx:
            binary += 1
    return binary


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__, file=sys.stderr)
        return 1

    func = sys.argv[1]
    target_addr_str = sys.argv[2]
    try:
        target_addr = int(target_addr_str, 16)
    except ValueError:
        print(f"ERROR: target address {target_addr_str!r} not a hex int", file=sys.stderr)
        return 1

    func_link_addr = get_func_link_address(func)
    if func_link_addr is None:
        print(f"ERROR: cannot find linked address for {func} (looked in named_syms.txt, undefined_syms_auto.txt, bb2.map)", file=sys.stderr)
        return 1

    target_offset = target_addr - func_link_addr
    if target_offset < 0:
        print(f"ERROR: target 0x{target_addr:08X} is BEFORE func 0x{func_link_addr:08X}", file=sys.stderr)
        return 1
    if target_offset % 4 != 0:
        print(f"WARNING: target offset 0x{target_offset:X} is not 4-aligned", file=sys.stderr)
    target_binary_idx = target_offset // 4

    print(f"# func {func} at 0x{func_link_addr:08X}")
    print(f"# target 0x{target_addr:08X} = offset 0x{target_offset:X} = binary idx {target_binary_idx}")

    insts, labels = parse_dump_text(func)
    if not insts:
        print(f"ERROR: dc.sh dump-text {func} returned no instructions", file=sys.stderr)
        return 1

    # Account for insert/insert_after/delete rules that change binary positions.
    inserts, insert_afters, deletes = parse_regfix_inserts_deletes(func)
    if inserts or insert_afters or deletes:
        parts = []
        if inserts: parts.append(f"{len(inserts)} insert(s) at {inserts}")
        if insert_afters: parts.append(f"{len(insert_afters)} insert_after(s) at {insert_afters}")
        if deletes: parts.append(f"{len(deletes)} delete(s) at {deletes}")
        print(f"# regfix deltas: " + ", ".join(parts))

    # Compute each label's binary position.
    label_to_pos: dict[str, tuple[int, int]] = {}  # label -> (text_idx, binary_idx)
    for text_idx_after, label in labels:
        b_idx = text_idx_to_binary_idx(insts, text_idx_after, inserts, insert_afters, deletes)
        label_to_pos[label] = (text_idx_after, b_idx)

    # Find label at target_binary_idx.
    matches = [(lbl, t, b) for lbl, (t, b) in label_to_pos.items() if b == target_binary_idx]
    if matches:
        for lbl, t, b in matches:
            print(f"MATCH: {lbl} (text idx {t}, binary idx {b})")
        print()
        print(f"Use this label name as the replacement in your subst rule.")
        return 0

    # No exact match. Find which text idx maps to target_binary_idx.
    # `insert_label @ N` puts the label AFTER text idx N — meaning the label
    # resolves to text idx (N+1)'s binary position. So to land the label at
    # target_binary_idx, we need (N+1)'s binary idx == target_binary_idx,
    # i.e., N is the idx whose NEXT instruction starts at target_binary_idx.
    suggested_idx: int | None = None
    target_text_idx: int | None = None
    prev_idx: int | None = None
    landed_inside_pseudo = False
    binary = 0
    for idx, _text, is_pseudo in insts:
        b = text_idx_to_binary_idx(insts, idx, inserts, insert_afters, deletes)
        binary = b
        if b == target_binary_idx:
            target_text_idx = idx
            suggested_idx = prev_idx if prev_idx is not None else idx - 1
            break
        if b > target_binary_idx:
            # target lands inside a pseudo expansion of the previous idx, OR
            # we walked past it (no valid landing). Take the closest before.
            target_text_idx = idx
            suggested_idx = prev_idx if prev_idx is not None else idx - 1
            landed_inside_pseudo = (prev_idx is not None)
            break
        prev_idx = idx

    if suggested_idx is None or target_text_idx is None:
        print(f"# No instruction at binary idx {target_binary_idx} (function ends at binary {binary})")
        return 1

    # Show closest existing labels for context
    nearby = sorted(label_to_pos.items(), key=lambda x: abs(x[1][1] - target_binary_idx))[:3]
    print(f"# Nearby labels:")
    for lbl, (t, b) in nearby:
        delta = b - target_binary_idx
        print(f"#   {lbl} (text idx {t}, binary idx {b}, delta {delta:+d} insts)")
    print()
    if landed_inside_pseudo:
        print(f"# WARNING: target binary idx {target_binary_idx} lands inside a pseudo expansion at text idx {target_text_idx}; insert_label cannot land mid-pseudo. Closest valid position chosen.")
    short = func.replace("func_", "F")[:8]
    sugg_label = f".LC{short}_X"
    print(f"SUGGEST: insert_label '{sugg_label}:' @ {suggested_idx}")
    print(f"         (label sits AFTER text idx {suggested_idx}, points to text idx {target_text_idx} = binary idx {target_binary_idx})")
    print(f"         then in your subst rule, use replacement: ...,{sugg_label}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
