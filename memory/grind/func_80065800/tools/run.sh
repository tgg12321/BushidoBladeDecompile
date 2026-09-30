#!/bin/bash
# Re-measure the func_80065800 landing package against current main (scratch TU only).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate
python3 memory/grind/func_80065800/tools/mk.py "${1:-memory/grind/func_80065800/candidate.c}" || exit 1
FUNCS="func_80065800 func_800645B0 func_800646E8"
for f in $(grep -lE 'D_800F0(B[A-C][0-9A-F]|C[A-F][0-9A-F]|D[0-7][0-9A-F])\b' asm/funcs/*.s); do
  n=$(basename "$f" .s); [ "$n" = func_80065800 ] || [ "$n" = func_800645B0 ] || [ "$n" = func_800646E8 ] || FUNCS="$FUNCS $n"
done
python3 memory/grind/func_80065800/tools/msbx.py tmp/f65800/re/tu1c.c - $FUNCS $2
