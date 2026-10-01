#!/bin/bash
# fr.sh <variant> <pseudo>... : dump with BB2_FINDREG_DEBUG for each pseudo, print the find_reg records.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
v="$1"; shift
bash tmp/func_80021DB0/r11tools/dump.sh tmp/func_80021DB0/$v.c tmp/func_80021DB0/d_$v "$@" > /dev/null 2>&1
for p in "$@"; do
  echo "== $v pseudo $p"
  cat tmp/func_80021DB0/d_$v/findreg_$p.txt
done
