#!/bin/bash
# Measure starting permuter score for a prepared backlog dir.
# Useful for triaging which candidates are closest to matching.
# Usage: ./bb2_measure_base_score.sh <permuter_dir>

set -eu
DIR="$1"
[ -d "$DIR" ] || { echo "ERROR: $DIR not found" >&2; exit 1; }
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

# Run permuter for 10 iterations to get base score
out=$(timeout 20 python3 tools/bb2_permuter.py "$DIR" --best-only -j 1 2>&1 | head -100)
# base score is the score on iteration 1 (or just the lowest in the first 10)
echo "$out" | tr "\r" "\n" | grep -oE "base score = [0-9]+|score = [0-9]+" | head -10 | sort -t'=' -k2 -n | head -1
