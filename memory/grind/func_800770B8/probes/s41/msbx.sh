#!/bin/bash
# msbx.sh <cand.c>...: engine sandbox --disable all on the private main clone /tmp/l770m/tree (main HEAD + f1C/f20
# unions). TAILN=n prints n lines of the per-hunk diff.
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd /tmp/l770m/tree || exit 1
source .venv/bin/activate
for c in "$@"; do
  case "$c" in /*) p="$c";; *) p="$R/$c";; esac
  out=$(python3 -m engine.cli sandbox func_800770B8 --disable all --diff --candidate "$p" 2>&1)
  s=$(echo "$out" | grep -E '"(score|build_insns)"' | tr -d ' \n,')
  echo "$(basename "$c"): $s"
  [ -n "$TAILN" ] && echo "$out" | sed -n '/insn diff/,$p' | head -"$TAILN"
done
git checkout -q -- metrics/events.jsonl 2>/dev/null
