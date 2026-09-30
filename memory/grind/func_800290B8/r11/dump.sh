#!/bin/bash
# Ruling 11 (D)(1) dumps for func_800290B8: compile the reuse spelling (candidate.c) and the
# one-variable-per-value spelling (r11/pv.c) inside src/code6cac_b.c with the build's cc1 flags
# (-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel
# -msoft-float) plus -dl -dg -df, cut out this function's section and print the allocator facts.
# Usage: bash memory/grind/func_800290B8/r11/dump.sh  (from the repo root)
set -e
R=/home/user/BushidoBladeDecompile
G=$R/memory/grind/func_800290B8
W=$R/tmp/w290; mkdir -p $W
cd $W
for v in candidate pv; do
  f=$G/r11/$v.c; [ $v = candidate ] && f=$G/candidate.c
  python3 $G/r11/cc.py $f $W/d_$v -dl -dg -df > /dev/null
  for d in lreg greg flow; do python3 $G/r11/sect.py $W/d_$v/t.i.$d > $W/d_$v/f.$d; done
done
python3 $G/r11/excerpt.py candidate pv
for v in candidate pv; do
  echo "=== $v: the bounding-box loop's i/2 (insn 52) and i&1 (insn 54), .lreg"
  grep -A2 "^(insn 5[24] " $W/d_$v/f.lreg
  echo "=== $v: local-alloc seats (.lreg ';; Register N in R.') for those pseudos"
  for p in $(grep -A1 "^(insn 5[24] " $W/d_$v/f.lreg | grep -o "reg/v:SI [0-9]*" | awk '{print $2}'); do
    grep "^;; Register $p in \|^Register $p used" $W/d_$v/f.lreg || true
  done
done
