#!/bin/bash
# test_on_current.sh : run land_all.sh on a scratch copy of main's CURRENT working tree (includes other lanes'
# staged edits) and flatten it to tmp/prc/cur/ for harness.py.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/prc/cur_tree; V=tmp/prc/cur
rm -rf $T $V && mkdir -p $T/src $T/include $V
cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
L1_ROOT=$T bash tmp/prc/land_all.sh
cp $T/src/code6cac_tu2.c $T/src/code6cac_c2.c $T/include/code6cac.h $V/
echo "chain applied on current tree"
