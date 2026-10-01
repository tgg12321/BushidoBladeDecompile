#!/bin/bash
# Allocation dumps + find_reg traces (BB2_FINDREG_DEBUG, instrumented tools/gcc-2.7.2/cc1) for the reuse
# body and the one-variable-per-value spelling. usage (repo root, WSL): bash .../r11/tools/dumps_all.sh <outroot> [pseudo...]
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT=${1:-tmp/e08}; shift || true
for v in reuse pv; do
  bash memory/grind/func_80074E08/r9/tools/dumps.sh memory/grind/func_80074E08/r11/variants/$v.c $OUT/r11_$v "$@" >/dev/null
  python3 memory/grind/func_80074E08/r9/tools/cut.py $OUT/r11_$v func_80074E08 >/dev/null
done
echo ok
