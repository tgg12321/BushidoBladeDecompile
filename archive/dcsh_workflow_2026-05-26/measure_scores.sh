#!/bin/bash
# Quick base-score check on prepared permuter dirs.
# Note: do NOT set -eu -- timeout's exit 124 from the permuter is expected
# and shouldn't kill the loop.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
source .venv/bin/activate
for f in "$@"; do
    if [ ! -d "permuter/$f" ]; then
        echo "$f: NOT PREPARED"
        continue
    fi
    out=$(timeout 25 python3 tools/bb2_permuter.py "permuter/$f" --best-only -j 1 2>&1 | tr '\r' '\n')
    base=$(echo "$out" | grep -oE 'base score = [0-9]+' | head -1 | awk '{print $NF}')
    minsc=$(echo "$out" | grep -oE 'score = [0-9]+' | awk '{print $NF}' | sort -n | head -1)
    iters=$(echo "$out" | grep -oE 'iteration [0-9]+' | awk '{print $NF}' | sort -n | tail -1)
    echo "$f: base=$base min=$minsc iters=$iters"
done
