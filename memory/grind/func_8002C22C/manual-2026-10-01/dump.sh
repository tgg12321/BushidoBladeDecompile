#!/bin/bash
# usage: dump.sh <tag> <override.c>   -> tmp/rtl/<tag>/
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
DEFS=$(make -s -f Makefile -pn 2>/dev/null | sed -n 's/^CPP_DEFS *:= *//p' | head -1)
mkdir -p tmp/func_8002C22C/i
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin $DEFS "$2" > tmp/func_8002C22C/i/$1.i
python3 tools/rtl_track/dump.py $1 --input tmp/func_8002C22C/i/$1.i > /dev/null
ls tmp/rtl/$1 | head -30
