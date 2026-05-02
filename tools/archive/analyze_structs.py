#!/usr/bin/env python3
"""
Analyze struct usage patterns to determine if a0/s0 represents one struct
or multiple different struct types.

Groups functions by the offsets they access, then clusters to find
distinct struct types.
"""
import re
from pathlib import Path
from collections import defaultdict, Counter

ROOT = Path(__file__).resolve().parent.parent


def get_func_access_pattern(asm_path):
    """Get all offsets accessed via a0/s0 in a function."""
    content = open(asm_path).read()
    offsets = {}

    for m in re.finditer(
        r'(lw|sw|lh|sh|lb|sb|lhu|lbu)\s+\$\w+,\s*(-?(?:0x[0-9a-fA-F]+|\d+))\(\$(a0|s0)\)',
        content
    ):
        instr, offset_str, base_reg = m.groups()
        offset = int(offset_str, 16) if '0x' in offset_str.lower() else int(offset_str)
        if 0 < offset < 8192:
            if offset not in offsets:
                offsets[offset] = set()
            offsets[offset].add(instr)

    return offsets


def main():
    funcs_dir = ROOT / "asm" / "funcs"

    # Gather per-function access patterns
    func_patterns = {}
    for asm in sorted(funcs_dir.glob("func_*.s")):
        pattern = get_func_access_pattern(asm)
        if pattern:
            func_patterns[asm.stem] = pattern

    print(f"Functions accessing a0/s0 with offsets: {len(func_patterns)}")

    # Find the max offset per function
    max_offsets = {}
    for func, offsets in func_patterns.items():
        max_offsets[func] = max(offsets.keys())

    # Bucket by max offset to see struct size distribution
    size_buckets = Counter()
    for func, max_off in max_offsets.items():
        bucket = (max_off // 128) * 128
        size_buckets[bucket] += 1

    print("\nStruct size distribution (max offset, 128-byte buckets):")
    for bucket in sorted(size_buckets.keys()):
        if size_buckets[bucket] >= 2:
            print(f"  0x{bucket:04X}-0x{bucket+127:04X}: {size_buckets[bucket]} functions")

    # Check: do functions accessing high offsets (>0x100) also access low offsets?
    # If yes, it's one big struct. If no, it might be different struct types.
    high_funcs = [f for f, m in max_offsets.items() if m > 0x100]
    low_access_count = 0
    for func in high_funcs:
        low_offsets = [o for o in func_patterns[func] if o < 0x20]
        if low_offsets:
            low_access_count += 1

    print(f"\nFunctions with max offset > 0x100: {len(high_funcs)}")
    print(f"  Also access offsets < 0x20: {low_access_count} ({100*low_access_count//max(len(high_funcs),1)}%)")

    # Find the densest offset range (most fields per 64-byte window)
    all_offsets = set()
    for pattern in func_patterns.values():
        all_offsets.update(pattern.keys())

    print(f"\nTotal unique offsets accessed: {len(all_offsets)}")

    # Determine field types by consensus across all functions
    field_types = defaultdict(lambda: defaultdict(int))
    for func, offsets in func_patterns.items():
        for off, instrs in offsets.items():
            for instr in instrs:
                field_types[off][instr] += 1

    # Build the definitive struct layout for offsets 0x00-0x100
    # These are the most commonly accessed and most important for m2c
    print("\n" + "="*70)
    print("DEFINITIVE FIELD TYPES (offsets 0x00-0x100)")
    print("="*70)

    for off in sorted(field_types.keys()):
        if off > 0x100:
            break
        instrs = field_types[off]
        total = sum(instrs.values())
        if total < 2:
            continue

        # Determine type: lw/sw = s32, lh/sh = s16, lhu = u16, lb/sb = s8, lbu = u8
        has_word = instrs.get('lw', 0) + instrs.get('sw', 0)
        has_half = instrs.get('lh', 0) + instrs.get('sh', 0)
        has_uhalf = instrs.get('lhu', 0)
        has_byte = instrs.get('lb', 0) + instrs.get('sb', 0)
        has_ubyte = instrs.get('lbu', 0)

        # If accessed as both word and halfword, it depends on alignment
        if off % 4 == 0 and has_word > 0:
            typ = 's32'
            size = 4
        elif off % 2 == 0 and (has_half > 0 or has_uhalf > 0):
            if has_uhalf > has_half:
                typ = 'u16'
            else:
                typ = 's16'
            size = 2
        elif has_byte > 0 or has_ubyte > 0:
            if has_ubyte > has_byte:
                typ = 'u8'
            else:
                typ = 's8'
            size = 1
        elif has_word > 0:
            typ = 's32'
            size = 4
        elif has_half > 0:
            typ = 's16'
            size = 2
        else:
            typ = 'u8'
            size = 1

        istr = ', '.join(f'{k}:{v}' for k, v in sorted(instrs.items()))
        print(f"  +0x{off:04X}: {typ:4s}  ({total:3d} accesses)  [{istr}]")


if __name__ == "__main__":
    main()
