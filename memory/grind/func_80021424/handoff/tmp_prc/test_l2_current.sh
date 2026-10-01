#!/bin/bash
# test_l2_current.sh : L1 chain + land_l2.py on a scratch copy of main's CURRENT working tree (other lanes' staged
# edits included), flattened to tmp/prc/l2cur/.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/prc/l2cur_tree; V=tmp/prc/l2cur
rm -rf $T $V && mkdir -p $T/src $T/include $V
cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
L1_ROOT=$T bash tmp/prc/land_all.sh > /dev/null
L1_ROOT=$T python3 tmp/prc/land_l2.py
cp $T/src/code6cac_tu2.c $T/src/code6cac_c2.c $T/include/code6cac.h $V/
grep -n "func_8003F3D4(" $V/code6cac_tu2.c | head -3
