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

WSL_PREFIX = [
    "wsl", "bash", "-c",
]
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
        f"python3 -m m2c --valid-syntax --target mipsel-gcc-2.7.2 {asm_path} 2>/dev/null"
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
    # Write m2c output to a temp file in WSL
    # We need to create a test file that includes common.h and the m2c code
    escaped_code = m2c_code.replace("'", "'\\''").replace("\\", "\\\\")

    cmd = (
        f"{WSL_CD} && "
        f"cat > /tmp/test_func.c << 'ENDOFFILE'\n"
        f"#include \"common.h\"\n"
        f"{m2c_code}\n"
        f"ENDOFFILE\n"
        f"tools/gcc-2.7.2/cc1 -O2 -G0 -quiet -mips1 -mcpu=3000 /tmp/test_func.c -o /dev/null 2>&1 | head -3"
    )
    try:
        result = subprocess.run(
            WSL_PREFIX + [cmd],
            capture_output=True, text=True, timeout=30
        )
        # If no error output, it compiled
        return len(result.stdout.strip()) == 0 and result.returncode == 0
    except (subprocess.TimeoutExpired, Exception):
        return False


def get_permuter_score(func_name, src_file):
    """Set up permuter and get initial score. Returns score or -1 on failure."""
    cmd = (
        f"{WSL_CD} && "
        f"bash tools/permuter_setup.sh {func_name} src/{src_file} 2>/dev/null && "
        f"python3 tools/decomp-permuter/permuter.py permuter/{func_name} --debug 2>&1 | head -5"
    )
    try:
        result = subprocess.run(
            WSL_PREFIX + [cmd],
            capture_output=True, text=True, timeout=120
        )
        # Parse score from permuter output
        for line in result.stdout.split('\n'):
            m = re.search(r'score\s*[=:]\s*(\d+)', line, re.IGNORECASE)
            if m:
                return int(m.group(1))
            # Also try "Best score: N" format
            m = re.search(r'(\d+)\s*\(', line)
            if m:
                return int(m.group(1))
        return -1
    except (subprocess.TimeoutExpired, Exception):
        return -1


def main():
    parser = argparse.ArgumentParser(description="Triage remaining stubs")
    parser.add_argument("--file", help="Only process stubs from this .c file")
    parser.add_argument("--dry-run", action="store_true", help="List stubs without processing")
    parser.add_argument("--skip-permuter", action="store_true", help="Skip permuter scoring (faster)")
    parser.add_argument("--resume", action="store_true", help="Skip functions already in results CSV")
    args = parser.parse_args()

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
