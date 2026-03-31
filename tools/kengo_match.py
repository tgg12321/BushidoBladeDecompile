#!/usr/bin/env python3
"""
kengo_match.py — Cross-reference BB2 stub sizes with Kengo function sizes.

Reads instruction counts from asm/funcs/*.s and size fields from
Kengo/kengo_functions_full.txt, then finds Kengo functions whose size
is within --tolerance of each BB2 stub's size.

By default, PS2-only Kengo modules are excluded (MPEG decoder, DVD driver,
TTY, etc.) because they have no equivalent in BB2's PS1 codebase and only
produce false-positive size coincidences. Use --include-ps2-only to see them.

Usage:
  python3 tools/kengo_match.py                           # all stubs
  python3 tools/kengo_match.py --file src/code6cac_b.c   # one BB2 file
  python3 tools/kengo_match.py --module is_motion        # filter Kengo module
  python3 tools/kengo_match.py --tolerance 0.20          # ±20% tolerance
  python3 tools/kengo_match.py --top 5                   # 5 candidates per stub
  python3 tools/kengo_match.py --csv out.csv             # also write CSV
  python3 tools/kengo_match.py --exact                   # exact size only
  python3 tools/kengo_match.py --include-ps2-only        # include PS2-only modules
"""

import re
import os
import sys
import csv
import glob
import argparse

BASE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# Kengo modules that exist only on PS2 and have no BB2 equivalent.
# These generate false-positive size matches and are excluded by default.
#
# mpc / pack  — PS2 IPU MPEG decoder (BB2 FMV is handled by MOVOVL.EXE overlay,
#               not an inline software decoder)
# eecdvd      — PS2 EE-side DVD/CD driver (BB2 uses PsyQ libcd BIOS calls)
# tty         — PS2 TTY debug terminal (no equivalent on PS1)
# init        — PS2 C-runtime / libc initialisation stubs
PS2_ONLY_MODULES = frozenset({
    "mpc",
    "pack",
    "eecdvd",
    "tty",
    "init",
})

# Per-BB2-file preferred Kengo module substrings for --use-affinity.
# Based on which Marionation subsystems are known or likely to appear in each
# BB2 source file.  A Kengo module is "preferred" if any entry here is a
# substring of its name (e.g. "sa_tan" matches sa_tan0, sa_tan1, …).
#
# When --use-affinity is active, candidates are first filtered to preferred
# modules.  If that leaves 0 results the tool falls back to all modules so
# nothing is silently dropped.
FILE_MODULE_AFFINITY = {
    "code6cac_b.c": [
        "is_motion", "is_pad", "is_action", "is_tanren",
        "nm_cpu", "nm_single_game",
        "is_coli", "is_ki_control",
        "sa_tan", "sa_se", "am_rmd",
    ],
    "code6cac.c": [
        "nm_camera", "nm_single_game", "nm_cpu",
        "md_game", "nm_mario", "is_motion",
    ],
    "code6cac_b2.c": [
        "nm_special_cam", "nm_mario_cam", "nm_replay_cam",
    ],
    "code6cac_c.c": [
        "is_coli", "is_ki_control", "is_damage_calc",
        "is_action", "is_motion", "is_pad",
    ],
    "code6cac_c2.c": [
        "nm_replay_cam", "nm_katinuki_game", "nm_single_game",
        "md_game", "am_rmd", "nm_cpu",
    ],
    "text1a.c": [
        "is_efc", "my_rob", "my_eff", "my_hirahira",
        "se_fc", "se_qt", "am_rmd",
    ],
    "main.c": [
        "md_game", "md_dummy", "is_learn", "is_league",
        "is_stats", "is_status", "is_replay", "is_rob_test",
        "am_rmd", "is_action", "is_coli", "sa_tan",
    ],
    "system.c": [
        "tsl_", "sa_se", "sa_load", "sa_main", "sa_eft",
    ],
    "ings.c": [
        "hi_curpad", "hi_gnd", "hi_landhit", "hi_gview", "hi_kgm", "common",
    ],
    "ings2.c": [
        "common",
    ],
    "config.c": [
        "md_option", "fade", "game_2d", "common",
    ],
    "sound.c": [
        "sa_se", "sa_load", "sa_main", "sa_tan",
    ],
}


def get_asm_insn_count(func_name):
    """Count real instructions in asm/funcs/<func_name>.s.

    Instruction lines have the format:
        /* offset addr bytes */  mnemonic  operands
    Labels, directives, blank lines, and comments are skipped.
    """
    path = os.path.join(BASE, "asm", "funcs", f"{func_name}.s")
    if not os.path.exists(path):
        return None
    count = 0
    insn_re = re.compile(r"^\s*/\* [0-9A-Fa-f]+ [0-9A-Fa-f]+ [0-9A-Fa-f]+ \*/")
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            if insn_re.match(line):
                count += 1
    return count


def get_bb2_stubs(filter_file=None):
    """Return list of (func_name, bb2_filename, insn_count) for all INCLUDE_ASM stubs."""
    stubs = []
    src_dir = os.path.join(BASE, "src")
    include_re = re.compile(r'INCLUDE_ASM\s*\(\s*"[^"]+"\s*,\s*(func_[0-9A-Fa-f]{8})\s*\)')

    for c_file in sorted(glob.glob(os.path.join(src_dir, "*.c"))):
        fname = os.path.basename(c_file)
        if filter_file and fname != filter_file:
            continue
        with open(c_file, encoding="utf-8", errors="replace") as f:
            content = f.read()
        for m in include_re.finditer(content):
            func_name = m.group(1)
            count = get_asm_insn_count(func_name)
            stubs.append((func_name, fname, count))

    return stubs


def get_kengo_functions(filter_module=None, exclude_modules=None):
    """Return list of (func_name, insn_count, source_file, module_name).

    kengo_functions_full.txt format:
        // FILE -- src/ishito/is_motion.c
        /* 00123456 000000a4 */ motion_SetMotion() {}
    The second hex field is the function size in bytes; divide by 4 for insns.

    Only functions whose source file begins with 'src/' are considered —
    this filters out PS2 SDK library code (/usr/local/sce/...) which has no
    BB2 counterpart.

    exclude_modules: set of module name strings to skip entirely.
    """
    path = os.path.join(BASE, "Kengo", "kengo_functions_full.txt")
    functions = []
    excluded_count = 0
    current_file = None
    in_game_file = False
    file_re = re.compile(r"// FILE -- (.+)")
    func_re = re.compile(r"/\* [0-9a-f]+ ([0-9a-f]+) \*/ (?:static )?(\w+)\(\)")

    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = file_re.match(line)
            if m:
                current_file = m.group(1).strip()
                # Only process game source files, not PS2 SDK libraries
                in_game_file = current_file.startswith("src/")
                continue
            if not in_game_file:
                continue
            m = func_re.match(line)
            if m and current_file:
                size_bytes = int(m.group(1), 16)
                if size_bytes == 0:
                    continue
                name = m.group(2)
                insn_count = size_bytes // 4
                module = os.path.splitext(os.path.basename(current_file))[0]
                if filter_module and filter_module not in module:
                    continue
                if exclude_modules and module in exclude_modules:
                    excluded_count += 1
                    continue
                functions.append((name, insn_count, current_file, module))

    return functions, excluded_count


def format_diff(diff):
    return f"{'+' if diff >= 0 else ''}{diff}"


def main():
    parser = argparse.ArgumentParser(
        description="Match BB2 stub sizes to Kengo function sizes"
    )
    parser.add_argument(
        "--file",
        help="Filter BB2 stubs to one source file (basename, e.g. code6cac_b.c)",
    )
    parser.add_argument(
        "--module",
        help="Filter Kengo candidates to module name substring (e.g. is_motion)",
    )
    parser.add_argument(
        "--tolerance",
        type=float,
        default=0.25,
        help="Fractional size tolerance (default 0.25 = ±25%%)",
    )
    parser.add_argument(
        "--exact",
        action="store_true",
        help="Only show exact-size matches (overrides --tolerance)",
    )
    parser.add_argument(
        "--top",
        type=int,
        default=5,
        help="Max Kengo candidates per stub (default 5)",
    )
    parser.add_argument(
        "--min-size",
        type=int,
        default=4,
        help="Skip BB2 stubs smaller than this many instructions (default 4)",
    )
    parser.add_argument(
        "--csv",
        metavar="FILE",
        help="Also write results to a CSV file",
    )
    parser.add_argument(
        "--single-match",
        action="store_true",
        help="Only show stubs with exactly one Kengo candidate (highest confidence)",
    )
    parser.add_argument(
        "--use-affinity",
        action="store_true",
        help=(
            "Filter candidates to preferred Kengo modules for each BB2 file "
            "(see FILE_MODULE_AFFINITY). Falls back to all modules if no "
            "preferred candidate exists. Combine with --single-match to find "
            "stubs that become unambiguous after affinity filtering."
        ),
    )
    parser.add_argument(
        "--include-ps2-only",
        action="store_true",
        help="Include PS2-only Kengo modules (mpc, pack, eecdvd, tty, init) — excluded by default",
    )
    parser.add_argument(
        "--no-header",
        action="store_true",
        help="Suppress the summary header",
    )
    args = parser.parse_args()

    if args.exact:
        args.tolerance = 0.0

    filter_file = os.path.basename(args.file) if args.file else None
    exclude_mods = None if args.include_ps2_only else PS2_ONLY_MODULES

    print("Loading BB2 stubs...", file=sys.stderr)
    bb2_stubs = get_bb2_stubs(filter_file)

    print("Loading Kengo functions...", file=sys.stderr)
    kengo_funcs, excluded_count = get_kengo_functions(args.module, exclude_mods)

    # Drop stubs with missing asm or below min size
    bb2_stubs = [
        (n, f, c) for n, f, c in bb2_stubs
        if c is not None and c >= args.min_size
    ]

    if not args.no_header:
        print(f"\nBB2 stubs:       {len(bb2_stubs)}")
        print(f"Kengo functions: {len(kengo_funcs)}", end="")
        if excluded_count:
            print(f"  ({excluded_count} PS2-only excluded — use --include-ps2-only to see them)", end="")
        print()
        tol_str = "exact" if args.exact else f"±{args.tolerance * 100:.0f}%"
        print(f"Tolerance:       {tol_str}")
        print()

    # Build sorted list by BB2 stub size descending (largest first = most informative)
    bb2_stubs.sort(key=lambda x: x[2], reverse=True)

    csv_rows = []
    match_count = 0
    no_match_count = 0

    for func_name, bb2_file, bb2_insns in bb2_stubs:
        if args.exact:
            candidates = [
                (name, insns, src_file, module)
                for name, insns, src_file, module in kengo_funcs
                if insns == bb2_insns
            ]
        else:
            lo = bb2_insns * (1.0 - args.tolerance)
            hi = bb2_insns * (1.0 + args.tolerance)
            candidates = [
                (name, insns, src_file, module)
                for name, insns, src_file, module in kengo_funcs
                if lo <= insns <= hi
            ]

        # Sort by absolute difference, then name for stability
        candidates.sort(key=lambda x: (abs(x[1] - bb2_insns), x[0]))

        # Apply module affinity filter if requested
        affinity_applied = False
        if args.use_affinity:
            preferred = FILE_MODULE_AFFINITY.get(bb2_file, [])
            if preferred:
                affinity_filtered = [
                    c for c in candidates
                    if any(p in c[3] for p in preferred)
                ]
                if affinity_filtered:
                    candidates = affinity_filtered
                    affinity_applied = True
                # else: no preferred candidates — fall back to all (candidates unchanged)

        candidates = candidates[: args.top]

        if args.single_match and len(candidates) != 1:
            if not candidates:
                no_match_count += 1
            continue

        if candidates:
            match_count += 1
            affinity_tag = "  [affinity]" if affinity_applied else ""
            print(f"{func_name}  [{bb2_file}]  {bb2_insns} insns{affinity_tag}")
            for name, insns, src_file, module in candidates:
                diff = insns - bb2_insns
                print(f"  → {name:<44} {insns:5d} insns ({format_diff(diff):>4})  [{module}]")
                csv_rows.append(
                    [func_name, bb2_file, bb2_insns, name, insns, diff, module, src_file]
                )
            print()
        else:
            no_match_count += 1
            csv_rows.append(
                [func_name, bb2_file, bb2_insns, "", "", "", "", ""]
            )

    print(f"Stubs with candidates: {match_count}")
    print(f"Stubs with no match:   {no_match_count}")

    if args.csv:
        with open(args.csv, "w", newline="", encoding="utf-8") as f:
            w = csv.writer(f)
            w.writerow([
                "bb2_func", "bb2_file", "bb2_insns",
                "kengo_name", "kengo_insns", "diff",
                "kengo_module", "kengo_source_file",
            ])
            w.writerows(csv_rows)
        print(f"\nCSV written to {args.csv}")


if __name__ == "__main__":
    main()
