#!/bin/bash
# mkl2.sh <name> [L2BODY] : scratch tree from HEAD's files + L1 chain + land_l2.py, flattened to tmp/prc/<name>/.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
N=$1; export L2BODY=${2:-l2r1}
T=tmp/prc/${N}_tree; V=tmp/prc/$N
rm -rf $T $V && mkdir -p $T/src $T/include $V
git show HEAD:src/code6cac_tu2.c > $T/src/code6cac_tu2.c
git show HEAD:src/code6cac_c2.c > $T/src/code6cac_c2.c
git show HEAD:include/code6cac.h > $T/include/code6cac.h
git show HEAD:undefined_syms_auto.txt > $T/undefined_syms_auto.txt
L1_ROOT=$T bash tmp/prc/land_all.sh > /dev/null
L1_ROOT=$T python3 tmp/prc/land_l2.py
cp $T/src/code6cac_tu2.c $T/src/code6cac_c2.c $T/include/code6cac.h $V/
