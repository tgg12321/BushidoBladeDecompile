#!/bin/bash
# m.sh <variant names...> : score tmp/func_8005E54C/v/<name>.c with match0 tu_patch, stripped and --nostrip
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TP=${TP:-memory/grind/func_8005E54C/match0/tu_patch.py}
for f in "$@"; do
  a=$(python3 tmp/func_8005E54C/sbxp.py tmp/func_8005E54C/v/$f.c $TP 2>&1 | tail -1)
  b=$(python3 tmp/func_8005E54C/sbxp.py tmp/func_8005E54C/v/$f.c $TP --nostrip 2>&1 | tail -1)
  echo "$f  strip=$a  nostrip=$b"
done
