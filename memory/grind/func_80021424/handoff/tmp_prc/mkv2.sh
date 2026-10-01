#!/bin/bash
# mkv2.sh <name> [BODY] : scratch tree = main's files + round3_full.patch + land_l1.py (bodies .<BODY>.c), flattened
# into tmp/prc/<name>/ for harness.py. Scratch only: nothing on main is written.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
N=$1; B=${2:-r3}
T=tmp/prc/${N}_tree; V=tmp/prc/$N
rm -rf $T $V && mkdir -p $T/src $T/include $V
cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
git apply --directory=$T --unsafe-paths tmp/func_80021424/round3_full.patch
L1_ROOT=$T BODY=$B python3 tmp/prc/land_l1.py
cp $T/src/code6cac_tu2.c $T/src/code6cac_c2.c $T/include/code6cac.h $V/
