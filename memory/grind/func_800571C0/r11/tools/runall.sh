#!/bin/bash
# runall.sh <variant>... : dump each tmp/func_800571C0/<variant>.c and print its pseudo table.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for v in "$@"; do
  bash tmp/func_800571C0/r11tools/dump.sh tmp/func_800571C0/$v.c tmp/func_800571C0/d_$v $FINDREG > /dev/null 2>&1
  echo "== $v ($(cat tmp/func_800571C0/d_$v/instcheck.txt))"
  python3 tmp/func_800571C0/r11tools/r11table.py tmp/func_800571C0/d_$v nl nr temp toL a ang
done
