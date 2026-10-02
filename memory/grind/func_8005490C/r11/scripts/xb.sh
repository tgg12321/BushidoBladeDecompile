#!/bin/bash
# usage: xb.sh <body.c|-> <tag> [--noA] [--c2]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
d=tmp/func_8005490C/x_$2
python3 tmp/func_8005490C/mk.py $1 $d $3 >/dev/null || exit 1
python3 tmp/func_8005490C/xbuild.py text1b $d/text1b.c $d/inc func_8005490C
if [ "$4" = "--c2" -o "$3" = "--c2" ]; then python3 tmp/func_8005490C/xbuild.py code6cac_c2 $d/code6cac_c2.c $d/inc; fi
