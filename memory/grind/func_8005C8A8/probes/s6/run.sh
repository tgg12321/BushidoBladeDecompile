#!/bin/bash
# run.sh name... : sandbox score + frame for each tmp/c8a8C/<name>.c
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for n in "$@"; do
  s=$(python3 tmp/c8a8C/sbx.py memory/grind/func_8005C8A8/probes/s6/$n.c 2>&1 | grep -m1 '"score"' | tr -dc '0-9')
  h=$(python3 tmp/c8a8C/sbx.py memory/grind/func_8005C8A8/probes/s6/$n.c 2>&1 | grep -m1 'source-level ·')
  echo "$n score=$s  $h"
done
