#!/bin/bash
# usage: run.sh [-q] <t-file...>   (paths relative to tmp/func_800207C8/)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
Q=""
if [ "$1" = "-q" ]; then Q="--q"; shift; fi
for t in "$@"; do
  b="${t%.t}"
  python3 tmp/func_800207C8/gen.py tmp/func_800207C8/$t tmp/func_800207C8/$b.c
  python3 tmp/func_800207C8/h.py tmp/func_800207C8/$b.c $(basename $b) $Q
done
