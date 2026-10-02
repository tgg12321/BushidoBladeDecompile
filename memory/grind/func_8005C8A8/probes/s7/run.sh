#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
python3 memory/grind/func_8005C8A8/probes/s5/setup.py
for f in "$@"; do
  s=$(python3 memory/grind/func_8005C8A8/probes/s5/sbx.py tmp/func_8005C8A8/s7/$f.c 2>&1)
  echo "== $f: $(echo "$s" | grep -iE '"score"|score|distance' | head -3 | tr '\n' ' ')"
  echo "$s" > tmp/func_8005C8A8/s7/$f.out
done
