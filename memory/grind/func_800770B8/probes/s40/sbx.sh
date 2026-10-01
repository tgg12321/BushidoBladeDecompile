#!/bin/bash
# sbx.sh <cand.c>...: engine sandbox --disable all in the private clone /tmp/l770/tree. TAILN lines of output (default: score only).
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd /tmp/l770/tree || exit 1
source .venv/bin/activate
for c in "$@"; do
  case "$c" in /*) p="$c";; *) p="$R/$c";; esac
  out=$(python3 -m engine.cli sandbox func_800770B8 --disable all --diff --candidate "$p" 2>&1)
  s=$(echo "$out" | grep -E '"(score|build_insns)"' | tr -d ' \n,')
  echo "$c: $s"
  [ -n "$TAILN" ] && echo "$out" | sed -n '/insn diff/,$p' | head -"$TAILN"
done
