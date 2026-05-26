#!/usr/bin/env python3
"""Diff GCC's loop-invariant constant hoisting (LICM) between mine and target.

Usage:
    python3 tools/diagnose_hoist.py <func_name>

When mine and target have the same C source but different callee-save register
allocations, GCC's LICM made different decisions about which constants to hoist.
This tool surfaces the difference and recommends a recipe:

  - constant X hoisted in mine but not target → defeat hoisting via inline-asm
    constant load (`__asm__ __volatile__("addiu %0,$0,X" : "=r"(c))`)
  - constant Y hoisted in target but not mine → force hoisting via
    `register int rY asm("$sN") = Y` and replace literal use sites with rY

Identifies hoisting by scanning the disassembled function for:
  - `addiu $sN, $zero, IMM`  (init of a callee-save with a constant)
  - matching `addu $aN, $sN, $zero` move sites that consume the hoisted reg

Output:
  func_70C70: hoist diff
    mine:   $s4 = 0x60   (1 init at +0x10, used 2x at +0x9C, +0x158)
    target: $s4 = 0x60   (1 init at +0x10, used 2x at +0xA4, +0x160) -- aligned
    mine:   $s5 = 261    (init at +0x?, used 1x at +0x?)             -- mine-only hoist
    target: $s7 = 0xC    (init at +0x?, used 3x at +0x?, ...)        -- target-only hoist

  Recipes to consider:
    - Defeat mine's $s5=261 hoist: `__asm__ __volatile__("addiu %0,$0,261":"=r"(c105));`
    - Force target's $s7=0xC hoist: `register s32 rC asm("$23") = 0xC;` then use rC

The output is advisory; you choose which recipe to apply.
"""
from __future__ import annotations

import os
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

# Callee-save registers in GCC's MIPS allocation order
CALLEE_SAVE_REGS = {
    "$s0": "$16", "$s1": "$17", "$s2": "$18", "$s3": "$19",
    "$s4": "$20", "$s5": "$21", "$s6": "$22", "$s7": "$23",
    "$fp": "$30",
}
# Reverse map for canonical printing
CANON = {v: k for k, v in CALLEE_SAVE_REGS.items()}
CANON.update({k: k for k in CALLEE_SAVE_REGS})


def find_o_for_func(func: str) -> Path | None:
    build_src = ROOT / "build" / "src"
    for o in sorted(build_src.glob("*.o")):
        try:
            r = subprocess.run(
                ["mipsel-linux-gnu-objdump", "-t", str(o)],
                capture_output=True, text=True, timeout=30,
            )
        except FileNotFoundError:
            return None
        if re.search(rf"\b{re.escape(func)}\b", r.stdout):
            return o
    return None


def disassemble_func_lines(o_path: Path, func: str) -> list[tuple[int, str]]:
    """Return [(byte_offset_within_func, asm_str), ...]"""
    r = subprocess.run(
        ["mipsel-linux-gnu-objdump", "-d", str(o_path)],
        capture_output=True, text=True, timeout=30,
    )
    out = []
    in_func = False
    func_base: int | None = None
    for line in r.stdout.splitlines():
        if re.match(rf"^[0-9a-f]+\s+<{re.escape(func)}>:", line):
            in_func = True
            func_base = int(line.split()[0], 16)
            continue
        if not in_func:
            continue
        if re.match(r"^[0-9a-f]+\s+<\w+>:", line):
            break
        m = re.match(r"^\s*([0-9a-f]+):\s+[0-9a-f]+\s+(.+)$", line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        offset = addr - (func_base or 0)
        asm = m.group(2).strip()
        out.append((offset, asm))
    return out


def parse_target_asm(func: str) -> list[tuple[int, str]]:
    """Read asm/funcs/<func>.s and return [(offset_in_func, asm_str), ...]"""
    p = ROOT / "asm" / "funcs" / f"{func}.s"
    if not p.exists():
        return []
    out = []
    base: int | None = None
    for line in p.read_text(encoding="utf-8").splitlines():
        # Lines look like: "    /* 6147C 80070C7C 21884000 */  addu       $s1, $a0, $zero"
        m = re.match(r"^\s*/\*\s+\w+\s+([0-9A-F]+)\s+\w+\s+\*/\s+(.+?)$", line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        if base is None:
            base = addr
        asm = m.group(2).strip()
        # strip trailing whitespace
        out.append((addr - base, asm))
    return out


def find_constant_inits(asm: list[tuple[int, str]]) -> dict[str, list[tuple[int, int]]]:
    """Find `addiu $sN/$fp, $zero, IMM` lines. Return {reg -> [(offset, value), ...]}"""
    inits: dict[str, list[tuple[int, int]]] = {}
    for offset, line in asm:
        # Match `addiu` or `li` against zero with imm
        # Examples:
        #   addiu $s4, $zero, 0x60
        #   li    s4, 96
        #   addiu $s4, $0, 96
        for pat in (
            r"^addiu\s+(\$\w+|\w+),\s*\$(?:zero|0)\s*,\s*(-?(?:0[xX][0-9a-fA-F]+|\d+))",
            r"^li\s+(\$\w+|\w+),\s*(-?(?:0[xX][0-9a-fA-F]+|\d+))",
        ):
            m = re.match(pat, line)
            if not m:
                continue
            reg = m.group(1)
            if not reg.startswith("$"):
                reg = "$" + reg
            canon = CANON.get(reg)
            if canon not in CALLEE_SAVE_REGS:
                continue
            try:
                val = int(m.group(2), 0)
            except ValueError:
                continue
            inits.setdefault(canon, []).append((offset, val))
            break
    return inits


def find_uses(asm: list[tuple[int, str]], reg_canon: str) -> list[int]:
    """Find offsets where reg_canon is read (not written-only).
    Approximate: any line that mentions the reg outside the destination position.
    """
    raw = CALLEE_SAVE_REGS[reg_canon]
    short = reg_canon
    uses: list[int] = []
    for offset, line in asm:
        # skip the init lines themselves
        if re.match(rf"^addiu\s+(\$\w+|\w+),\s*\$(?:zero|0)\s*,", line):
            # only skip if dest is this reg
            m = re.match(r"^addiu\s+(\$\w+|\w+)", line)
            if m:
                d = m.group(1)
                if not d.startswith("$"):
                    d = "$" + d
                if CANON.get(d) == reg_canon:
                    continue
        # skip save/restore lines
        if re.match(rf"^(sw|lw)\s+(\$\w+|\w+),", line):
            m = re.match(r"^(sw|lw)\s+(\$\w+|\w+)", line)
            if m:
                d = m.group(2)
                if not d.startswith("$"):
                    d = "$" + d
                if CANON.get(d) == reg_canon and "($sp)" in line.replace(" ", ""):
                    continue
        # any other mention of $sN (as src or dest)
        if re.search(rf"\$(s[0-7]|fp|{raw[1:]})\b", line):
            # Resolve mention to canonical
            for tok in re.findall(r"\$\w+", line):
                if CANON.get(tok) == reg_canon:
                    uses.append(offset)
                    break
    return uses


def main() -> int:
    if len(sys.argv) != 2:
        print(__doc__, file=sys.stderr)
        return 1
    func = sys.argv[1]

    o = find_o_for_func(func)
    if not o:
        print(f"ERROR: cannot find .o file for {func}", file=sys.stderr)
        return 1

    mine = disassemble_func_lines(o, func)
    target = parse_target_asm(func)
    if not target:
        print(f"ERROR: target asm not found at asm/funcs/{func}.s", file=sys.stderr)
        return 1

    mine_inits = find_constant_inits(mine)
    target_inits = find_constant_inits(target)

    print(f"# {func}: hoist diff")
    print(f"#   mine .o:    {o}")
    print(f"#   target asm: asm/funcs/{func}.s")
    print()

    # Build sets of (reg, value) tuples present
    mine_set = set()
    for r, lst in mine_inits.items():
        for off, val in lst:
            mine_set.add((r, val))
    target_set = set()
    for r, lst in target_inits.items():
        for off, val in lst:
            target_set.add((r, val))

    # Aligned (same in both)
    aligned = mine_set & target_set
    mine_only = mine_set - target_set
    target_only = target_set - mine_set

    if aligned:
        print("# aligned hoists (mine and target both initialize this reg with this constant):")
        for reg, val in sorted(aligned):
            mu = find_uses(mine, reg)
            print(f"#   {reg} = {val} (0x{val:X})   mine uses at offsets {[hex(x) for x in mu[:5]]}")
        print()

    if mine_only:
        print("# MINE-ONLY hoists (target uses literal addiu at the use site):")
        for reg, val in sorted(mine_only):
            uses = find_uses(mine, reg)
            print(f"#   mine: {reg} = {val} (0x{val:X})   {len(uses)} use(s) at {[hex(x) for x in uses[:5]]}")
        print(f"#   RECIPE: defeat the hoist by inline-asm-loading the constant at each use site:")
        for reg, val in sorted(mine_only):
            cname = f"c{val}_{reg.replace('$','')}"
            print(f"#     {{ s32 {cname}; __asm__ __volatile__(\"addiu %0,$0,{val}\" : \"=r\"({cname})); /* use {cname} */ }}")
        print()

    if target_only:
        print("# TARGET-ONLY hoists (mine uses literal addiu, target hoists to a callee-save):")
        for reg, val in sorted(target_only):
            print(f"#   target: {reg} = {val} (0x{val:X})")
        print(f"#   RECIPE: force GCC to hoist by `register asm`:")
        for reg, val in sorted(target_only):
            num = CALLEE_SAVE_REGS[reg].lstrip("$")
            cname = f"r{val:X}"
            print(f"#     register s32 {cname} asm(\"${num}\") = 0x{val:X};   /* then use {cname} where {val} appears */")
        print()

    if not (aligned or mine_only or target_only):
        print("# No callee-save constant inits found in either side. Hoisting is not the issue.")
        return 0

    # Sanity counts
    print(f"# Summary: {len(aligned)} aligned, {len(mine_only)} mine-only, {len(target_only)} target-only")
    return 0


if __name__ == "__main__":
    sys.exit(main())
