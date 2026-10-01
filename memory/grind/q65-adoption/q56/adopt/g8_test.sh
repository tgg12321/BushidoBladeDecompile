#!/bin/bash
# g8_test.sh: Q69 proof in the scratch clone at its head (step16). Builds three ways and compares every object:
#   A = the series (maspsx -G8 for the files whose code reaches gp: SDATA_FILES)
#   B = -G8 for every file except Sony library code (libfiles.py LIBRARY set)
#   C = -G8 for every file (global)
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
cd "/tmp/q56/adopt tree" && source .venv/bin/activate
LIB="text1b_b_tu2 text1b_b_tu3 gpu display system ings2 main comb"
ALL=$(ls src/*.c | xargs -n1 basename | sed 's/\.c$//' | tr '\n' ' ')
NONLIB=$(for f in $ALL; do case " $LIB " in *" $f "*) ;; *) printf '%s ' $f;; esac; done)
build() { rm -rf build; make -j16 build/bb2.exe > /tmp/q56/g8_$1.log 2>&1; echo "$1: $(sha1sum build/bb2.exe 2>/dev/null | cut -c1-40) $(grep -c -i error /tmp/q56/g8_$1.log) error lines"; rm -rf /tmp/q56/g8obj_$1; cp -r build/src /tmp/q56/g8obj_$1; }
cmpobj() { n=0; for o in /tmp/q56/g8obj_A/*.o; do b=$(basename $o); cmp -s $o /tmp/q56/g8obj_$1/$b || { n=$((n+1)); echo "   differs vs A: $b"; }; done; echo "  $1 vs A: $n objects differ"; }
build A
sed -i "s/^SDATA_FILES := .*/SDATA_FILES := $NONLIB/" Makefile; build B; cmpobj B
sed -i "s/^SDATA_FILES := .*/SDATA_FILES := $ALL/" Makefile; build C; cmpobj C
grep -m3 -i "error\|discarded" /tmp/q56/g8_C.log
git checkout -q -- Makefile
