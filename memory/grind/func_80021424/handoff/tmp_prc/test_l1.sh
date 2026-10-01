#!/bin/bash
# Scratch-test land_l1.py: copy main's four files to tmp/prc/t_tree, apply round3_full.patch there (scratch only),
# run land_l1.py against it, and compare with the measured variant tree tmp/prc/r3_tree.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/prc/t_tree
rm -rf $T && mkdir -p $T/src $T/include
cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
git apply --directory=$T --unsafe-paths tmp/func_80021424/round3_full.patch
L1_ROOT=$T python3 tmp/prc/land_l1.py
diff $T/include/code6cac.h tmp/prc/r3_tree/include/code6cac.h && diff $T/src/code6cac_tu2.c tmp/prc/r3_tree/src/code6cac_tu2.c && echo SAME_AS_MEASURED
grep -n "D_80101F08\|D_80101F10\|D_80101F14 \|D_80101F42" $T/undefined_syms_auto.txt || true
