#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
for n in "$@"; do
  python3 tmp/b60/sbx.py tmp/b60/$n.c > tmp/b60/$n.out 2>&1
  printf '%-14s %s  %s %s\n' "$n" "$(grep -m1 '"score"' tmp/b60/$n.out)" "$(grep -m1 'source-level ·' tmp/b60/$n.out)" "$(grep -m1 -i 'error' tmp/b60/$n.out)"
done
