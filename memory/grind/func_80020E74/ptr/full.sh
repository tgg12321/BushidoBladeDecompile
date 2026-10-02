#!/bin/bash
# full.sh <dir>: per-function compare (code6cac_tu2 code6cac ings) + private full link
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
rm -rf $1/obj
python3 tmp/func_80020E74/dm.py $1 code6cac_tu2 code6cac ings 2>&1 | grep -v "funcs compared"
python3 tmp/func_80020E74/plink.py $1 2>&1 | tail -2
