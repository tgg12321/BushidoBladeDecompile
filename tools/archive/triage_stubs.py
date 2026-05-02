"""Triage all remaining INCLUDE_ASM stubs: run m2c, try compile, get permuter score.

Usage:
    python tools/triage_stubs.py                    # all remaining stubs
    python tools/triage_stubs.py --file text1a.c    # one file only
    python tools/triage_stubs.py --dry-run           # list without processing

Output: triage_results.csv with columns:
    func_name, src_file, asm_lines, m2c_compiles, permuter_score, local_draft_exists
"""
import argparse
import csv
import os
import re
import subprocess
import sys
import time

SRC_DIR = "src"
ASM_DIR = os.path.join("asm", "funcs")
DRAFT_DIR = "local_drafts"
RESULTS_FILE = "triage_results.csv"

# Detect if we're already in WSL/Linux or running from Windows
IN_WSL = sys.platform == "linux"

if IN_WSL:
    WSL_PREFIX = ["bash", "-c"]
    WSL_CD = 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate'
else:
    WSL_PREFIX = ["wsl", "bash", "-c"]
    WSL_CD = 'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate'


def find_stubs(src_filter=None):
    """Find all INCLUDE_ASM stubs across src/ files."""
    stubs = []
    for fname in sorted(os.listdir(SRC_DIR)):
        if not fname.endswith(".c"):
            continue
        if src_filter and fname != src_filter:
            continue
        fpath = os.path.join(SRC_DIR, fname)
        with open(fpath) as f:
            for line in f:
                m = re.search(r'INCLUDE_ASM\("asm/funcs",\s*(func_[0-9A-Fa-f]+)\)', line)
                if m:
                    stubs.append((fname, m.group(1)))
    return stubs


def get_asm_lines(func_name):
    path = os.path.join(ASM_DIR, f"{func_name}.s")
    if not os.path.exists(path):
        return 0
    with open(path) as f:
        return sum(1 for _ in f)


def run_m2c(func_name):
    """Run m2c on a function, return (success, c_code)."""
    asm_path = f"asm/funcs/{func_name}.s"
    cmd = (
        f"{WSL_CD} && "
        f"m2c --valid-syntax --target mipsel-gcc-c {asm_path} 2>/dev/null"
    )
    try:
        result = subprocess.run(
            WSL_PREFIX + [cmd],
            capture_output=True, text=True, timeout=60
        )
        if result.returncode == 0 and result.stdout.strip():
            return True, result.stdout.strip()
        return False, ""
    except (subprocess.TimeoutExpired, Exception):
        return False, ""


def try_compile_m2c(func_name, src_file, m2c_code):
    """Try to compile m2c output in a temp file, return success."""
    # Write the test file with proper headers via python (avoids heredoc issues)
    test_content = '#include "common.h"\n#include "m2c_macros.h"\n' + m2c_code + '\n'
    test_path = os.path.join("m2c_triage", f"{func_name}.c")
    os.makedirs("m2c_triage", exist_ok=True)
    with open(test_path, "w", newline='\n') as f:
        f.write(test_content)

    wsl_path = f"/mnt/c/Users/Trenton/Desktop/\"Bushido Blade 2 Decompile\"/m2c_triage/{func_name}.c"
    cmd = (
        f"{WSL_CD} && "
        f"mipsel-linux-gnu-cpp -Iinclude -Itools/m2c {wsl_path} 2>/dev/null | "
        f"tools/gcc-2.7.2/cc1 -O2 -G0 -quiet -mips1 -mcpu=3000 -o /dev/null 2>&1 | "
        f"grep -c 'error\\|parse error'"
    )
    try:
        result = subprocess.run(
            WSL_PREFIX + [cmd],
            capture_output=True, text=True, timeout=30
        )
        # grep -c returns the count of error lines; "0" means no errors
        return result.stdout.strip() == "0"
    except (subprocess.TimeoutExpired, Exception):
        return False


def get_permuter_score(func_name, src_file):
    """Set up permuter and get initial score. Returns score or -1 on failure."""
    # Check if we have an m2c output to use as base.c
    m2c_path = os.path.join("m2c_triage", f"{func_name}.c")
    if not os.path.exists(m2c_path):
        return -1

    with open(m2c_path) as f:
        m2c_code = f.read()

    # Set up permuter directory, then overwrite base.c with m2c output
    cmd = (
        f"{WSL_CD} && "
        f"bash tools/permuter_setup.sh {func_name} src/{src_file} 2>/dev/null"
    )
    try:
        subprocess.run(WSL_PREFIX + [cmd], capture_output=True, text=True, timeout=60)
    except (subprocess.TimeoutExpired, Exception):
        return -1

    # Overwrite base.c with the m2c output, replacing m2c_macros.h include
    # with inline macro since permuter's include path doesn't cover tools/m2c/
    base_path = os.path.join("permuter", func_name, "base.c")
    if not os.path.exists(os.path.dirname(base_path)):
        return -1

    # Replace all includes with inlined types — the permuter's own preprocessor
    # can't find project headers, so we must inline everything
    code = m2c_code.replace('#include "m2c_macros.h"\n', '')
    code = code.replace('#include "common.h"\n', '')
    inline_header = (
        'typedef unsigned char u8;\n'
        'typedef signed char s8;\n'
        'typedef unsigned short u16;\n'
        'typedef signed short s16;\n'
        'typedef unsigned int u32;\n'
        'typedef signed int s32;\n'
        'typedef unsigned long long u64;\n'
        'typedef signed long long s64;\n'
        '#define NULL ((void *)0)\n'
        '#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))\n'
        'typedef s32 M2C_UNK;\n'
        '\n'
    )
    code = inline_header + code

    with open(base_path, "w", newline='\n') as f:
        f.write(code)

    # Run permuter in debug mode to get score
    cmd2 = (
        f"{WSL_CD} && "
        f"python3 tools/decomp-permuter/permuter.py permuter/{func_name} --debug 2>&1 | tail -5"
    )
    try:
        result = subprocess.run(
            WSL_PREFIX + [cmd2],
            capture_output=True, text=True, timeout=120
        )
        # Parse score from permuter output — format: "[func_XXX] base score = N"
        for line in result.stdout.split('\n'):
            m = re.search(r'base score\s*=\s*(\d+)', line)
            if m:
                return int(m.group(1))
            m = re.search(r'score\s*[=:]\s*(\d+)', line, re.IGNORECASE)
            if m:
                return int(m.group(1))
        return -1
    except (subprocess.TimeoutExpired, Exception):
        return -1


def run_score_only():
    """Re-read triage_results.csv, score any row that compiled but has no permuter score."""
    if not os.path.exists(RESULTS_FILE):
        print(f"Error: {RESULTS_FILE} not found. Run full triage first.")
        sys.exit(1)

    with open(RESULTS_FILE) as f:
        rows = list(csv.DictReader(f))

    to_score = [r for r in rows if r['m2c_compiles'] == 'Y' and (r['permuter_score'] == '' or r['permuter_score'] == '-1')]
    print(f"Found {len(to_score)} compiled stubs needing permuter scores (out of {len(rows)} total)")

    for i, row in enumerate(to_score):
        func_name = row['func_name']
        src_file = row['src_file']
        print(f"[{i+1}/{len(to_score)}] {func_name} ({row['asm_lines']} lines, {src_file})...", end=" ", flush=True)

        score = get_permuter_score(func_name, src_file)
        row['permuter_score'] = score if score >= 0 else ''

        status = f"score:{score}" if score >= 0 else "FAIL"
        print(status)

    # Rewrite the full CSV
    with open(RESULTS_FILE, 'w', newline='') as csvfile:
        fieldnames = ['func_name', 'src_file', 'asm_lines', 'm2c_parses', 'm2c_compiles',
                      'permuter_score', 'local_draft', 'tier']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)

    # Summary
    scored = [r for r in rows if r['permuter_score'] not in ('', '-1')]
    low = sum(1 for r in scored if int(r['permuter_score']) < 500)
    zero = sum(1 for r in scored if int(r['permuter_score']) == 0)
    print(f"\n=== SUMMARY ===")
    print(f"Scored:         {len(scored)}")
    print(f"  Score < 500:  {low} (grindable)")
    print(f"  Score = 0:    {zero} (already matching!)")
    print(f"Results updated in {RESULTS_FILE}")


def main():
    parser = argparse.ArgumentParser(description="Triage remaining stubs")
    parser.add_argument("--file", help="Only process stubs from this .c file")
    parser.add_argument("--dry-run", action="store_true", help="List stubs without processing")
    parser.add_argument("--skip-permuter", action="store_true", help="Skip permuter scoring (faster)")
    parser.add_argument("--resume", action="store_true", help="Skip functions already in results CSV")
    parser.add_argument("--score-only", action="store_true", help="Only score functions that compiled but have no permuter score")
    args = parser.parse_args()

    if args.score_only:
        run_score_only()
        return

    stubs = find_stubs(args.file)

    # Load existing results if resuming
    done = set()
    if args.resume and os.path.exists(RESULTS_FILE):
        with open(RESULTS_FILE) as f:
            reader = csv.DictReader(f)
            for row in reader:
                done.add(row['func_name'])

    if args.dry_run:
        print(f"Found {len(stubs)} stubs:")
        for src_file, func_name in stubs:
            size = get_asm_lines(func_name)
            draft = "Y" if os.path.exists(os.path.join(DRAFT_DIR, f"{func_name}.c")) else "N"
            print(f"  {func_name} ({size} lines) from {src_file} [draft: {draft}]")
        return

    # Open CSV for writing
    write_header = not args.resume or not os.path.exists(RESULTS_FILE)
    mode = 'a' if args.resume else 'w'

    with open(RESULTS_FILE, mode, newline='') as csvfile:
        fieldnames = ['func_name', 'src_file', 'asm_lines', 'm2c_parses', 'm2c_compiles',
                      'permuter_score', 'local_draft', 'tier']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        if write_header:
            writer.writeheader()

        total = len(stubs)
        for i, (src_file, func_name) in enumerate(stubs):
            if func_name in done:
                continue

            asm_lines = get_asm_lines(func_name)
            has_draft = os.path.exists(os.path.join(DRAFT_DIR, f"{func_name}.c"))

            print(f"[{i+1}/{total}] {func_name} ({asm_lines} lines, {src_file})...", end=" ", flush=True)

            # Run m2c
            m2c_ok, m2c_code = run_m2c(func_name)
            compiles = False
            if m2c_ok:
                compiles = try_compile_m2c(func_name, src_file, m2c_code)

            # Permuter score (only if m2c compiled and not skipped)
            score = -1
            if compiles and not args.skip_permuter:
                score = get_permuter_score(func_name, src_file)

            # Classify tier
            if asm_lines < 25:
                tier = "SMALL"
            elif asm_lines <= 80:
                tier = "MEDIUM"
            elif asm_lines <= 150:
                tier = "LARGE"
            else:
                tier = "EXPERT"

            status = f"m2c:{'OK' if m2c_ok else 'FAIL'} compile:{'OK' if compiles else 'FAIL'}"
            if score >= 0:
                status += f" score:{score}"

            print(status)

            writer.writerow({
                'func_name': func_name,
                'src_file': src_file,
                'asm_lines': asm_lines,
                'm2c_parses': 'Y' if m2c_ok else 'N',
                'm2c_compiles': 'Y' if compiles else 'N',
                'permuter_score': score if score >= 0 else '',
                'local_draft': 'Y' if has_draft else 'N',
                'tier': tier,
            })
            csvfile.flush()

    print(f"\nResults saved to {RESULTS_FILE}")

    # Print summary
    with open(RESULTS_FILE) as f:
        reader = list(csv.DictReader(f))
        total = len(reader)
        m2c_ok = sum(1 for r in reader if r['m2c_parses'] == 'Y')
        compiles = sum(1 for r in reader if r['m2c_compiles'] == 'Y')
        scored = [r for r in reader if r['permuter_score'] and r['permuter_score'] != '']
        low_score = sum(1 for r in scored if int(r['permuter_score']) < 500)
        zero_score = sum(1 for r in scored if int(r['permuter_score']) == 0)

        print(f"\n=== SUMMARY ===")
        print(f"Total stubs:      {total}")
        print(f"m2c parses:       {m2c_ok}/{total} ({100*m2c_ok//total}%)")
        print(f"m2c compiles:     {compiles}/{total} ({100*compiles//total}%)")
        if scored:
            print(f"Permuter scored:  {len(scored)}")
            print(f"  Score < 500:    {low_score} (grindable)")
            print(f"  Score = 0:      {zero_score} (already matching!)")


if __name__ == "__main__":
    main()
