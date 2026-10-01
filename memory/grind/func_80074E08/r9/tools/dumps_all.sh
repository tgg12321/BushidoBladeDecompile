#!/bin/bash
# Allocation dumps for the reuse body and the one-local-per-write spellings (repo root, WSL).
# usage: bash memory/grind/func_80074E08/r9/tools/dumps_all.sh <outroot>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT=${1:-tmp/e08}
for v in reuse pv_fn pv_block nolocal; do
  bash memory/grind/func_80074E08/r9/tools/dumps.sh memory/grind/func_80074E08/r9/variants/$v.c $OUT/fd_$v >/dev/null
  python3 memory/grind/func_80074E08/r9/tools/cut.py $OUT/fd_$v func_80074E08 >/dev/null
done
echo ok
