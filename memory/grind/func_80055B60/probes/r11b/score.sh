#!/bin/bash
# score.sh <name>...: score tmp/func_80055B60/r11/<name>.c as func_80055B60 (text1b with the landing header + src edits)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
python3 tmp/func_80055B60/hdr.py tmp/func_80055B60/inc/include/code6cac.h tmp/func_80055B60/d6a/include/code6cac.h >/dev/null || exit 1
for n in "$@"; do
  python3 tmp/func_80055B60/mksrc.py tmp/func_80055B60/r11/$n.c tmp/func_80055B60/r11/tu_$n.c >/dev/null || { echo "$n MKSRC-FAIL"; continue; }
  out=$(python3 memory/grind/func_80055B60/probes/prep_d6a78/tucheck.py text1b tmp/func_80055B60/r11/tu_$n.c tmp/func_80055B60/inc/include 2>&1)
  s=$(echo "$out" | grep "DIFF func_80055B60" | awk '{print $3}')
  e=$(echo "$out" | grep -m1 -i "error")
  o=$(echo "$out" | grep "^DIFF" | grep -v func_80055B60 | tr '\n' ' ')
  printf '%-16s %s %s %s\n' "$n" "${s:-0}" "$e" "$o"
  rm -f tmp/func_80055B60/r11/tu_$n.c
done
