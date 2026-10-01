#!/bin/bash
# Dumps + find_reg traces for the reuse body and the three one-variable-per-value spellings.
# usage (repo root, WSL): bash memory/grind/func_8002CD58/r11/tools/dumps_all.sh <outroot>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT=${1:-tmp/cd58r11}
for v in reuse pv_both pv_len pv_temp; do
  bash memory/grind/func_8002CD58/r11/tools/dumps.sh memory/grind/func_8002CD58/r11/variants/$v.c $OUT/fd_$v 126 127 78 >/dev/null
  python3 memory/grind/func_8002CD58/r11/tools/cut.py $OUT/fd_$v func_8002CD58 >/dev/null
done
echo ok
