"""Compare local model drafts by attempting to compile each one.

Reads drafts from local_drafts/<model>/<func>.c (A/B test layout) and
tries to compile each with the project's GCC 2.7.2 pipeline. Reports
which model produces more compilable output.

Usage:
    python tools/compare_drafts.py                    # compare all models
    python tools/compare_drafts.py --func func_80040304  # one function
    python tools/compare_drafts.py --models bb2-deepseek bb2-codestral

Requires WSL with the build toolchain set up.
"""
import argparse
import csv
import os
import subprocess
import sys

DRAFTS_DIR = "local_drafts"
MODELS = ["bb2-deepseek", "bb2-codestral"]

# Minimal wrapper that includes project headers and compiles a draft
COMPILE_WRAPPER = r'''
#include "common.h"
#include "include_asm.h"

{code}
'''

# WSL compile command — just check if it compiles, don't need to link
COMPILE_CMD = r'''
cd /mnt/c/Users/Trenton/Desktop/"Bushido Blade 2 Decompile"
INPUT="/tmp/bb2_compare_input.c"
cat > "$INPUT" << 'CEOF'
{wrapped_code}
CEOF
mipsel-linux-gnu-cpp \
  -I./include -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
  "$INPUT" 2>/dev/null \
  | ./tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w 2>/dev/null \
  | python3 ./tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=./sdata_syms.txt \
      --sdata-funcs=./sdata_funcs.txt \
      --sdata-exclude=./sdata_exclude.txt 2>/dev/null \
  | mipsel-linux-gnu-as -I./include -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o /tmp/bb2_compare_output.o 2>/dev/null
echo $?
'''


def try_compile(code):
    """Try to compile a draft. Returns True if successful."""
    wrapped = COMPILE_WRAPPER.format(code=code)
    cmd = COMPILE_CMD.format(wrapped_code=wrapped.replace("'", "'\\''"))

    try:
        result = subprocess.run(
            ["wsl", "bash", "-c", cmd],
            capture_output=True, text=True, timeout=30
        )
        exit_code = result.stdout.strip().split('\n')[-1]
        return exit_code == "0"
    except (subprocess.TimeoutExpired, Exception):
        return False


def find_functions(models, func_filter=None):
    """Find all functions that have drafts from any model."""
    funcs = set()
    for model in models:
        model_dir = os.path.join(DRAFTS_DIR, model)
        if not os.path.isdir(model_dir):
            continue
        for fname in os.listdir(model_dir):
            if fname.endswith(".c"):
                func_name = fname[:-2]
                if func_filter and func_name != func_filter:
                    continue
                funcs.add(func_name)
    return sorted(funcs)


def main():
    parser = argparse.ArgumentParser(description="Compare model drafts by compile-testing")
    parser.add_argument("--models", nargs="+", default=MODELS, help="Models to compare")
    parser.add_argument("--func", help="Test only this function")
    parser.add_argument("--limit", type=int, default=0, help="Max functions to test (0 = all)")
    args = parser.parse_args()

    models = args.models
    funcs = find_functions(models, args.func)

    if not funcs:
        print("No drafts found. Run A/B test first:")
        print("  python tools/batch_local_decomp.py --ab-test")
        return

    if args.limit > 0:
        funcs = funcs[:args.limit]

    print(f"Comparing {len(funcs)} functions across {len(models)} models\n")
    print(f"{'Function':<25}", end="")
    for m in models:
        print(f" {m:<20}", end="")
    print()
    print("-" * (25 + 20 * len(models)))

    # Track results
    scores = {m: {"pass": 0, "fail": 0, "missing": 0} for m in models}
    rows = []

    for func in funcs:
        row = {"function": func}
        print(f"{func:<25}", end="", flush=True)

        for model in models:
            draft_path = os.path.join(DRAFTS_DIR, model, f"{func}.c")
            if not os.path.exists(draft_path):
                print(f" {'—':^20}", end="")
                row[model] = "missing"
                scores[model]["missing"] += 1
                continue

            with open(draft_path) as f:
                code = f.read()

            ok = try_compile(code)
            if ok:
                print(f" {'PASS':^20}", end="")
                row[model] = "pass"
                scores[model]["pass"] += 1
            else:
                print(f" {'FAIL':^20}", end="")
                row[model] = "fail"
                scores[model]["fail"] += 1

        print()
        rows.append(row)

    # Summary
    print(f"\n{'='*60}")
    print("SUMMARY")
    print(f"{'='*60}")
    print(f"\n{'Model':<20} {'Pass':>6} {'Fail':>6} {'Missing':>8} {'Rate':>8}")
    print("-" * 50)
    for m in models:
        s = scores[m]
        total = s["pass"] + s["fail"]
        rate = f"{s['pass']/total*100:.0f}%" if total > 0 else "n/a"
        print(f"{m:<20} {s['pass']:>6} {s['fail']:>6} {s['missing']:>8} {rate:>8}")

    # Determine winner
    rates = {}
    for m in models:
        s = scores[m]
        total = s["pass"] + s["fail"]
        rates[m] = s["pass"] / total if total > 0 else 0

    if rates:
        best = max(rates, key=rates.get)
        print(f"\nBest compile rate: {best} ({rates[best]*100:.0f}%)")

    # Save CSV
    csv_path = os.path.join(DRAFTS_DIR, "compile_comparison.csv")
    with open(csv_path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=["function"] + models)
        writer.writeheader()
        writer.writerows(rows)
    print(f"Results saved to {csv_path}")


if __name__ == "__main__":
    main()
