#!/usr/bin/env python3
"""
Apply matched functions from auto_matches/ back into source files.

For each file in auto_matches/func_XXXX.c:
  1. Find which source file contains the INCLUDE_ASM for that function
  2. Extract just the function definition from the matched code
  3. Replace the INCLUDE_ASM line with the function code
  4. Build and verify (optional)

Usage:
  python3 tools/apply_matches.py                   # apply all, dry-run
  python3 tools/apply_matches.py --apply            # apply all for real
  python3 tools/apply_matches.py --func func_XXXX   # apply one function
  python3 tools/apply_matches.py --apply --verify    # apply + build + verify
  python3 tools/apply_matches.py --list              # list available matches
"""
import os, re, sys, subprocess, argparse, csv
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
MATCHES_DIR = ROOT / "auto_matches"
SRC_DIR = ROOT / "src"
TRIAGE_CSV = ROOT / "triage_results.csv"


def find_source_file(func_name):
    """Find which .c file contains the INCLUDE_ASM for a function."""
    # First check triage_results.csv for the mapping
    if TRIAGE_CSV.exists():
        with open(TRIAGE_CSV) as f:
            reader = csv.DictReader(f)
            for row in reader:
                if row["func"] == func_name:
                    return SRC_DIR / row["file"]

    # Fallback: grep all source files
    for src in sorted(SRC_DIR.glob("*.c")):
        with open(src) as f:
            content = f.read()
        if f'INCLUDE_ASM("asm/funcs", {func_name})' in content:
            return src

    return None


def extract_function_code(match_file):
    """Extract usable function code from an auto_matches file.

    Strips typedefs and extern declarations, returning just the function
    definition and any needed externs/forward declarations.
    """
    with open(match_file) as f:
        content = f.read()

    lines = content.strip().split('\n')
    result_lines = []
    in_func = False
    brace_depth = 0

    # Skip typedefs that are already in the project headers
    skip_types = {'typedef unsigned char u8;', 'typedef signed char s8;',
                  'typedef unsigned short u16;', 'typedef signed short s16;',
                  'typedef unsigned int u32;', 'typedef signed int s32;'}

    for line in lines:
        stripped = line.strip()

        # Skip our injected typedefs
        if stripped.rstrip(';') + ';' in skip_types or stripped in skip_types:
            continue

        # Skip empty lines before function
        if not in_func and not stripped:
            continue

        # Detect function start
        if not in_func and re.match(r'^(void|int|u32|s32|u16|s16|u8|s8|long|short|char|unsigned)\s+func_', stripped):
            in_func = True

        if in_func:
            result_lines.append(line)
            brace_depth += line.count('{') - line.count('}')
            if brace_depth == 0 and '{' in ''.join(result_lines):
                break
        else:
            # Keep extern declarations — they may be needed
            if stripped.startswith('extern ') or (stripped.endswith(';') and 'func_' in stripped and '(' in stripped):
                result_lines.append(line)

    return '\n'.join(result_lines)


def apply_match(func_name, match_file, dry_run=True):
    """Replace an INCLUDE_ASM with the matched function code."""
    src_file = find_source_file(func_name)
    if not src_file:
        print(f"  ERROR: No source file found containing INCLUDE_ASM for {func_name}")
        return False

    if not src_file.exists():
        print(f"  ERROR: Source file {src_file} does not exist")
        return False

    with open(src_file) as f:
        content = f.read()

    # Find the INCLUDE_ASM line
    pattern = f'INCLUDE_ASM("asm/funcs", {func_name});'
    if pattern not in content:
        print(f"  SKIP: {func_name} not found in {src_file.name} (already decompiled?)")
        return False

    # Get the function code
    func_code = extract_function_code(match_file)
    if not func_code.strip():
        print(f"  ERROR: Could not extract function code from {match_file}")
        return False

    if dry_run:
        print(f"  DRY RUN: Would replace INCLUDE_ASM in {src_file.name}")
        print(f"    {pattern}")
        print(f"    → {func_code.split(chr(10))[0]}... ({func_code.count(chr(10))+1} lines)")
        return True

    # Replace the INCLUDE_ASM with the function code
    new_content = content.replace(pattern, func_code)

    with open(src_file, 'w', newline='\n') as f:
        f.write(new_content)

    print(f"  APPLIED: {func_name} → {src_file.name}")
    return True


def verify_build():
    """Run make and check for success."""
    print("\nBuilding...")
    result = subprocess.run(
        ["wsl", "bash", "-c",
         'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && '
         'source .venv/bin/activate && make 2>&1 | tail -5'],
        capture_output=True, text=True, timeout=120
    )
    output = result.stdout.strip()
    print(output)
    return "OK: bb2 matches!" in output


def list_matches():
    """List all available matches."""
    if not MATCHES_DIR.exists():
        print("No auto_matches/ directory found.")
        return

    matches = sorted(MATCHES_DIR.glob("func_*.c"))
    if not matches:
        print("No matches found in auto_matches/")
        return

    print(f"{len(matches)} matches available:\n")
    for match_file in matches:
        func_name = match_file.stem
        src_file = find_source_file(func_name)
        src_name = src_file.name if src_file else "???"

        # Check if already applied
        if src_file and src_file.exists():
            with open(src_file) as f:
                content = f.read()
            already = f'INCLUDE_ASM("asm/funcs", {func_name})' not in content
        else:
            already = False

        status = "APPLIED" if already else "pending"
        lines = open(match_file).read().count('\n') + 1
        print(f"  {func_name:20s}  {src_name:16s}  {lines:4d} lines  [{status}]")


def main():
    parser = argparse.ArgumentParser(description="Apply matched functions from auto_matches/ into source files")
    parser.add_argument("--apply", action="store_true", help="Actually apply changes (default: dry run)")
    parser.add_argument("--verify", action="store_true", help="Build and verify after applying")
    parser.add_argument("--func", help="Apply only this function (e.g., func_80043C7C)")
    parser.add_argument("--list", action="store_true", help="List available matches")
    args = parser.parse_args()

    if args.list:
        list_matches()
        return

    if not MATCHES_DIR.exists():
        print("No auto_matches/ directory found. Run smart_match.py first.")
        return

    matches = sorted(MATCHES_DIR.glob("func_*.c"))
    if args.func:
        matches = [m for m in matches if m.stem == args.func]

    if not matches:
        print("No matches to apply.")
        return

    dry_run = not args.apply
    if dry_run:
        print(f"DRY RUN — showing what would be applied ({len(matches)} matches)")
        print("Run with --apply to actually apply.\n")
    else:
        print(f"Applying {len(matches)} matches...\n")

    applied = 0
    skipped = 0
    for match_file in matches:
        func_name = match_file.stem
        print(f"[{func_name}]")
        if apply_match(func_name, match_file, dry_run=dry_run):
            applied += 1
        else:
            skipped += 1

    print(f"\n{'='*60}")
    action = "Would apply" if dry_run else "Applied"
    print(f"{action}: {applied}  Skipped: {skipped}")

    if args.verify and args.apply and applied > 0:
        if verify_build():
            print("\nBuild verified successfully!")
        else:
            print("\nBuild FAILED — check errors above.")


if __name__ == "__main__":
    main()
