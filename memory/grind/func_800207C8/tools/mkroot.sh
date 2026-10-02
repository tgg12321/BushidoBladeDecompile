#!/bin/bash
# mkroot.sh <mode> : copy the tree's header + 3 src files to tmp/func_800207C8/root, apply edits, mirror into srcmod
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
R=tmp/func_800207C8/root
rm -rf $R; mkdir -p $R/include $R/src
cp include/code6cac.h $R/include/
cp src/code6cac_b_tu2.c src/code6cac_tu2.c src/code6cac.c $R/src/
python3 tmp/func_800207C8/apply.py $R $1
rm -rf tmp/func_800207C8/srcmod; mkdir -p tmp/func_800207C8/srcmod
cp $R/src/*.c tmp/func_800207C8/srcmod/
HDR_FILE=$R/include/code6cac.h NOCAND=1 python3 tmp/func_800207C8/chk.py x | grep -v " 0 differences$"
bash tmp/func_800207C8/pl.sh | tail -1
