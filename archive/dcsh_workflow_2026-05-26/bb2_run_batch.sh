#!/bin/bash
# Run the targeted permuter on a list of prepared candidates serially.
# For each: stop on score=0; if a match found, capture the source.c.
# Usage: ./bb2_run_batch.sh <time_per_func> <func> [<func>...]

set -u
TIME="$1"
shift
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
source .venv/bin/activate

for f in "$@"; do
    permdir="permuter/$f"
    if [ ! -d "$permdir" ]; then
        echo "$f: NOT PREPARED -- skipping"
        continue
    fi
    # Clear prior outputs (we want a fresh search)
    rm -rf "$permdir"/output-*
    log="/tmp/batch_${f}.log"
    BB2_PERMUTER_HEAVY=1 timeout "$TIME" python3 tools/bb2_permuter.py \
        "$permdir" --stop-on-zero --best-only -j 6 > "$log" 2>&1 || true
    minsc=$(grep -oE 'score = [0-9]+' "$log" | awk '{print $NF}' | sort -n | head -1)
    iters=$(grep -oE 'iteration [0-9]+' "$log" | awk '{print $NF}' | sort -n | tail -1)
    if [ -d "$permdir"/output-0-1 ]; then
        echo "$f: MATCH (min=$minsc iters=$iters)"
    else
        echo "$f: best=$minsc iters=$iters"
    fi
done
