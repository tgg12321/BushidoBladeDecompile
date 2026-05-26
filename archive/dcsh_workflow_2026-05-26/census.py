#!/usr/bin/env python3
"""Census of remaining INCLUDE_ASM stubs by difficulty tier and file."""

import re
import os
import glob


def get_func_size(func_name):
    """Get function size in instructions from its .s file."""
    sf = f"asm/funcs/{func_name}.s"
    if not os.path.exists(sf):
        return 0
    with open(sf) as f:
        lines = f.readlines()
    # Count instruction lines (lines with hex address pattern)
    count = 0
    for line in lines:
        stripped = line.strip()
        if re.match(r"/\*\s+[0-9a-fA-F]+\s+8[0-9a-fA-F]{7}\s+[0-9a-fA-F]+\s+\*/", stripped):
            count += 1
    return count


def main():
    c_files = sorted(glob.glob("src/*.c"))

    total_stubs = 0
    total_decompiled = 0
    tiers = {"tiny": [], "small": [], "medium": [], "large": [], "expert": []}
    file_stats = {}

    for cf in c_files:
        basename = os.path.basename(cf)
        with open(cf) as f:
            content = f.read()

        # Count INCLUDE_ASM stubs
        stubs = re.findall(r'INCLUDE_ASM\("asm/funcs",\s*(\w+)\)', content)

        # Count decompiled functions (C function definitions)
        # Simple heuristic: lines matching function signature pattern
        decomp_count = len(re.findall(r'^(?:void|s32|u32|s16|u16|s8|u8|int|unsigned)\s+func_', content, re.MULTILINE))

        stub_tiers = {"tiny": 0, "small": 0, "medium": 0, "large": 0, "expert": 0}

        for func_name in stubs:
            size = get_func_size(func_name)
            if size <= 5:
                tier = "tiny"
            elif size <= 15:
                tier = "small"
            elif size <= 30:
                tier = "medium"
            elif size <= 80:
                tier = "large"
            else:
                tier = "expert"
            tiers[tier].append((func_name, size, basename))
            stub_tiers[tier] += 1

        total_stubs += len(stubs)
        total_decompiled += decomp_count
        file_stats[basename] = {
            "stubs": len(stubs),
            "decompiled": decomp_count,
            "tiers": stub_tiers,
        }

    # Print summary
    print("=" * 70)
    print(f"DECOMPILATION CENSUS")
    print(f"Total INCLUDE_ASM stubs: {total_stubs}")
    print(f"Total decompiled functions: {total_decompiled}")
    print(f"Completion: {total_decompiled}/{total_stubs + total_decompiled} "
          f"({100*total_decompiled/(total_stubs+total_decompiled):.1f}%)")
    print("=" * 70)

    print(f"\nBy tier:")
    for tier_name, tier_list in tiers.items():
        print(f"  {tier_name:8s} (instructions): {len(tier_list):4d} stubs")

    tier_ranges = {
        "tiny": "1-5 instr",
        "small": "6-15 instr",
        "medium": "16-30 instr",
        "large": "31-80 instr",
        "expert": "81+ instr",
    }

    print(f"\nBy file:")
    print(f"{'File':<16s} {'Stubs':>6s} {'Decomp':>6s} {'Tiny':>5s} {'Small':>5s} {'Med':>5s} {'Large':>5s} {'Expert':>6s}")
    print("-" * 66)
    for fname in sorted(file_stats.keys()):
        s = file_stats[fname]
        t = s["tiers"]
        print(f"{fname:<16s} {s['stubs']:>6d} {s['decompiled']:>6d} "
              f"{t['tiny']:>5d} {t['small']:>5d} {t['medium']:>5d} {t['large']:>5d} {t['expert']:>6d}")

    # List tiny/small functions for quick wins
    print(f"\n{'='*70}")
    print(f"QUICK WINS: {len(tiers['tiny']) + len(tiers['small'])} functions (tiny + small)")
    print(f"{'='*70}")
    for tier_name in ["tiny", "small"]:
        for func_name, size, fname in sorted(tiers[tier_name], key=lambda x: x[1]):
            print(f"  {func_name:<24s} {size:>3d} instr  ({fname})")


if __name__ == "__main__":
    main()
