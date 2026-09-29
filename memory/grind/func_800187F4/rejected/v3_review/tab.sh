#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate 2>/dev/null
dirs=""
for t in "$@"; do bash tmp/rv3/alloc2.sh $t; dirs="$dirs tmp/func_800187F4/dump_$t"; done
python3 memory/grind/func_800187F4/r11/tools/r11table.py $dirs
