"""Grind permuter on top-scoring functions from triage results.

Usage (run in WSL/tmux):
    python3 tools/permuter_grind.py                      # top 20 by score
    python3 tools/permuter_grind.py --max-score 500      # only grindable (<500)
    python3 tools/permuter_grind.py --max-score 1000     # include moderate
    python3 tools/permuter_grind.py --timeout 600        # 10 min per function (default: 300s)
    python3 tools/permuter_grind.py --func func_8001EEB4 # single function
    python3 tools/permuter_grind.py --dry-run             # show plan without running

Results saved to permuter_grind_results.csv and per-function best outputs in permuter/<func>/
"""
import argparse
import csv
import os
import re
import subprocess
import sys
import time

TRIAGE_FILE = "triage_results.csv"
RESULTS_FILE = "permuter_grind_results.csv"
M2C_DIR = "m2c_triage"
PERMUTER_DIR = "permuter"

INLINE_HEADER = """\
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned long long u64;
typedef signed long long s64;
#define NULL ((void *)0)
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
typedef s32 M2C_UNK;

"""


def get_candidates(max_score, limit, single_func=None):
    """Read triage CSV and return candidates sorted by score."""
    if not os.path.exists(TRIAGE_FILE):
        print(f"Error: {TRIAGE_FILE} not found. Run triage_stubs.py first.")
        sys.exit(1)

    with open(TRIAGE_FILE) as f:
        rows = list(csv.DictReader(f))

    if single_func:
        return [r for r in rows if r['func_name'] == single_func]

    candidates = []
    for r in rows:
        if r['permuter_score'] in ('', '-1'):
            continue
        score = int(r['permuter_score'])
        if score <= max_score and score > 0:
            candidates.append(r)

    candidates.sort(key=lambda r: int(r['permuter_score']))
    return candidates[:limit]


def setup_permuter(func_name, src_file):
    """Set up permuter directory with m2c output as base."""
    m2c_path = os.path.join(M2C_DIR, f"{func_name}.c")
    if not os.path.exists(m2c_path):
        return False

    # Run permuter_setup.sh to create target.s/target.o/compile.sh
    cmd = f'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && source .venv/bin/activate && bash tools/permuter_setup.sh {func_name} src/{src_file} 2>/dev/null'
    subprocess.run(["bash", "-c", cmd], capture_output=True, text=True, timeout=60)

    base_path = os.path.join(PERMUTER_DIR, func_name, "base.c")
    if not os.path.exists(os.path.dirname(base_path)):
        return False

    # Read m2c output and inline headers
    with open(m2c_path) as f:
        m2c_code = f.read()

    code = m2c_code.replace('#include "m2c_macros.h"\n', '')
    code = code.replace('#include "common.h"\n', '')
    code = INLINE_HEADER + code

    with open(base_path, "w", newline='\n') as f:
        f.write(code)

    return True


def run_permuter(func_name, timeout):
    """Run permuter with timeout. Returns (best_score, matched)."""
    cmd = (
        f'cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile" && '
        f'source .venv/bin/activate && '
        f'timeout {timeout} python3 tools/decomp-permuter/permuter.py '
        f'permuter/{func_name} -j4 --stop-on-zero --best-only 2>&1'
    )
    try:
        result = subprocess.run(
            ["bash", "-c", cmd],
            capture_output=True, text=True, timeout=timeout + 30
        )
        output = result.stdout + result.stderr

        # Check for match (score 0)
        if 'score = 0' in output or 'score 0' in output:
            return 0, True

        # Parse best score — look for "score = N" patterns, take the lowest
        best = None
        for m in re.finditer(r'score\s*=\s*(\d+)', output):
            s = int(m.group(1))
            if best is None or s < best:
                best = s

        return best if best is not None else -1, False
    except subprocess.TimeoutExpired:
        # Timeout is expected — permuter grinds until killed
        # Parse whatever output we got
        return -1, False
    except Exception as e:
        print(f" error: {e}")
        return -1, False


def main():
    parser = argparse.ArgumentParser(description="Grind permuter on top candidates")
    parser.add_argument("--max-score", type=int, default=1000, help="Max initial score to attempt (default: 1000)")
    parser.add_argument("--limit", type=int, default=20, help="Max functions to process (default: 20)")
    parser.add_argument("--timeout", type=int, default=300, help="Seconds per function (default: 300)")
    parser.add_argument("--func", help="Single function to grind")
    parser.add_argument("--dry-run", action="store_true", help="Show plan without running")
    args = parser.parse_args()

    candidates = get_candidates(args.max_score, args.limit, args.func)

    if not candidates:
        print("No candidates found.")
        return

    print(f"=== Permuter Grind ===")
    print(f"Candidates:  {len(candidates)}")
    print(f"Max score:   {args.max_score}")
    print(f"Timeout:     {args.timeout}s per function")
    print(f"Threads:     4")
    print()

    if args.dry_run:
        for r in candidates:
            print(f"  {r['func_name']:30s} score={r['permuter_score']:>5s}  ({r['asm_lines']} lines, {r['src_file']})")
        return

    matches = []
    improved = []

    # Append mode — load existing results to skip already-done functions
    done = set()
    if os.path.exists(RESULTS_FILE):
        with open(RESULTS_FILE) as f:
            for row in csv.DictReader(f):
                done.add(row['func_name'])

    write_header = not os.path.exists(RESULTS_FILE) or os.path.getsize(RESULTS_FILE) == 0
    with open(RESULTS_FILE, 'a', newline='') as csvfile:
        writer = csv.DictWriter(csvfile, fieldnames=[
            'func_name', 'src_file', 'asm_lines', 'initial_score', 'final_score', 'matched', 'time_s'
        ])
        if write_header:
            writer.writeheader()

        for i, r in enumerate(candidates):
            func_name = r['func_name']
            src_file = r['src_file']
            initial = int(r['permuter_score'])

            if func_name in done:
                print(f"[{i+1}/{len(candidates)}] {func_name} — already done, skipping")
                continue

            print(f"[{i+1}/{len(candidates)}] {func_name} (score={initial}, {r['asm_lines']} lines)...", flush=True)

            # Setup
            if not setup_permuter(func_name, src_file):
                print(f"  Setup failed, skipping")
                continue

            # Grind
            t0 = time.time()
            final_score, matched = run_permuter(func_name, args.timeout)
            elapsed = time.time() - t0

            if matched:
                print(f"  *** MATCH! *** ({elapsed:.0f}s)")
                matches.append(func_name)
            elif final_score >= 0 and final_score < initial:
                print(f"  Improved: {initial} -> {final_score} ({elapsed:.0f}s)")
                improved.append((func_name, initial, final_score))
            else:
                print(f"  No improvement: {initial} -> {final_score if final_score >= 0 else 'error'} ({elapsed:.0f}s)")

            writer.writerow({
                'func_name': func_name,
                'src_file': src_file,
                'asm_lines': r['asm_lines'],
                'initial_score': initial,
                'final_score': final_score,
                'matched': 'Y' if matched else 'N',
                'time_s': f"{elapsed:.0f}",
            })
            csvfile.flush()

    # Summary
    print(f"\n=== RESULTS ===")
    print(f"Processed:  {len(candidates)}")
    print(f"Matches:    {len(matches)}")
    print(f"Improved:   {len(improved)}")
    if matches:
        print(f"\nMatched functions:")
        for f in matches:
            print(f"  {f}")
    if improved:
        print(f"\nImproved functions:")
        for f, old, new in improved:
            print(f"  {f}: {old} -> {new}")
    print(f"\nResults saved to {RESULTS_FILE}")


if __name__ == "__main__":
    main()
