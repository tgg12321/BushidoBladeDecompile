#!/bin/bash
# usage: bash tmp/func_80070188/sw.sh base.c variants.py outdir [extra sc.py flags]
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
base=$1; spec=$2; out=$3; shift 3
list=$(python3 tmp/func_80070188/gen.py "$base" "$spec" "$out") || exit 1
python3 tmp/func_80070188/sc.py --mini "$@" $(echo "$list" | tr ',' ' ')
