#!/usr/bin/env python3
"""
Infer struct layouts from asm access patterns and decompiled code.

Scans all asm/funcs/*.s files for load/store instructions with register+offset
addressing, then groups by base register to identify struct field layouts.

Also scans decompiled src/*.c files for pointer+offset access patterns.

Usage: python3 tools/infer_structs.py
"""
import re
from pathlib import Path
from collections import defaultdict, Counter

ROOT = Path(__file__).resolve().parent.parent


def scan_asm_stubs():
    """Scan asm stubs for register+offset access patterns."""
    # reg -> offset -> {(instr_type, count)}
    patterns = defaultdict(lambda: defaultdict(lambda: defaultdict(int)))

    for asm in sorted((ROOT / "asm" / "funcs").glob("func_*.s")):
        content = open(asm).read()
        # Match: lw $reg, OFFSET($base) — offset can be decimal or hex (0x...)
        # Also match negative offsets like -0x20($sp)
        for m in re.finditer(
            r'(lw|sw|lh|sh|lb|sb|lhu|lbu)\s+\$\w+,\s*(-?(?:0x[0-9a-fA-F]+|\d+))\(\$(\w+)\)',
            content
        ):
            instr, offset_str, base_reg = m.groups()
            offset = int(offset_str, 16) if '0x' in offset_str.lower() else int(offset_str)
            if 0 < offset < 8192:
                patterns[base_reg][offset][instr] += 1

    return patterns


def scan_decompiled_code():
    """Scan decompiled C for pointer+offset access patterns."""
    # var_name -> offset -> types
    patterns = defaultdict(lambda: defaultdict(set))

    for src in sorted((ROOT / "src").glob("*.c")):
        content = open(src).read()
        # Match: *(type *)(var + 0xOFFSET)
        for m in re.finditer(
            r'\*(s32|u32|s16|u16|s8|u8)\s*\*\)\s*\(\s*(?:\(u8\s*\*\))?\s*(\w+)\s*\+\s*0x([0-9a-fA-F]+)\)',
            content
        ):
            typ, var, offset_hex = m.groups()
            offset = int(offset_hex, 16)
            patterns[var][offset].add(typ)

    return patterns


def infer_type_from_instr(instr_counts):
    """Infer C type from instruction usage."""
    total = sum(instr_counts.values())
    # Prioritize by most common access type
    if 'lh' in instr_counts or 'sh' in instr_counts:
        if 'lhu' in instr_counts:
            return 'u16'
        return 's16'
    if 'lb' in instr_counts or 'sb' in instr_counts:
        if 'lbu' in instr_counts:
            return 'u8'
        return 's8'
    if 'lw' in instr_counts or 'sw' in instr_counts:
        return 's32'
    return 's32'


def main():
    print("Scanning asm stubs...")
    asm_patterns = scan_asm_stubs()

    print("Scanning decompiled C code...")
    c_patterns = scan_decompiled_code()

    # Focus on the most commonly accessed base registers
    # a0 = first arg (usually main struct pointer)
    # s0 = often a0 saved to callee-saved register
    for reg in ['a0', 's0', 'a1', 's1', 'a2', 's2']:
        if reg not in asm_patterns:
            continue
        offsets = asm_patterns[reg]
        if len(offsets) < 5:
            continue

        print(f"\n{'='*60}")
        print(f"Struct accessed via ${reg} ({len(offsets)} fields)")
        print(f"{'='*60}")

        for off in sorted(offsets.keys()):
            instr_counts = offsets[off]
            total = sum(instr_counts.values())
            typ = infer_type_from_instr(instr_counts)
            instrs = ', '.join(f'{k}:{v}' for k, v in sorted(instr_counts.items()))
            print(f"  +0x{off:04X} ({off:5d}): {typ:4s}  [{instrs}]  ({total} accesses)")

    # Show struct inference from decompiled C
    print(f"\n{'='*60}")
    print("Structs from decompiled C (pointer+offset patterns)")
    print(f"{'='*60}")
    for var in sorted(c_patterns.keys()):
        offsets = c_patterns[var]
        if len(offsets) < 5:
            continue
        print(f"\n  {var} ({len(offsets)} fields):")
        for off in sorted(offsets.keys()):
            types = ', '.join(sorted(offsets[off]))
            print(f"    +0x{off:03X}: {types}")

    # Cross-reference: find overlap between a0/s0 patterns
    a0_offs = set(asm_patterns.get('a0', {}).keys())
    s0_offs = set(asm_patterns.get('s0', {}).keys())
    overlap = a0_offs & s0_offs
    if overlap:
        print(f"\n{'='*60}")
        print(f"a0/s0 overlap ({len(overlap)} shared offsets) — likely same struct")
        print(f"{'='*60}")

        # Merge and print unified struct
        merged = defaultdict(lambda: defaultdict(int))
        for off in sorted(a0_offs | s0_offs):
            for reg in ['a0', 's0']:
                if off in asm_patterns.get(reg, {}):
                    for instr, count in asm_patterns[reg][off].items():
                        merged[off][instr] += count

        print("\ntypedef struct {")
        prev_end = 0
        for off in sorted(merged.keys()):
            typ = infer_type_from_instr(merged[off])
            size = {'s8': 1, 'u8': 1, 's16': 2, 'u16': 2, 's32': 4, 'u32': 4}[typ]
            if off > prev_end:
                gap = off - prev_end
                if gap > 0:
                    print(f"    /* 0x{prev_end:04X} */ u8 pad_{prev_end:04X}[{gap}];")
            total = sum(merged[off].values())
            print(f"    /* 0x{off:04X} */ {typ} field_{off:04X}; /* {total} accesses */")
            prev_end = off + size
        print("} GameStruct;")


if __name__ == "__main__":
    main()
