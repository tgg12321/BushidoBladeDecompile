#!/bin/bash
# perm_control.sh: the permuter scorer's base score for the reuse body (control: must be 0) through the
# same compile.sh / target.o as the campaign (perm_setup.sh)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
P=tmp/func_80057E84/r11/perm
C=tmp/func_80057E84/r11/perm_control
mkdir -p $C
cp $P/compile.sh $P/settings.toml $P/target.o $C/
cp $P/reuse_control.c $C/base.c
bash $C/compile.sh $C/base.c -o $C/base.o
timeout 60 python3 tools/decomp-permuter/permuter.py $C -j 1 --stop-on-zero 2>&1 | grep -m1 "base score"
