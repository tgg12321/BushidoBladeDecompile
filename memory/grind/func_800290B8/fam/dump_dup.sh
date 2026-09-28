#!/bin/bash
# Allocator dumps for the multi-block loop-1 probes (vtx statement duplicated into arms) and, for
# contrast, the block-local pv / dw_init bodies. Build cc1 flags + -dl -dg -df (r11/cc.py).
# Usage: bash memory/grind/func_800290B8/fam/dump_dup.sh  (from the repo root)
set -e
R=/home/user/BushidoBladeDecompile
G=$R/memory/grind/func_800290B8
W=$R/tmp/w290; mkdir -p $W
cd $W
for v in dw_init dup_vtx_col dup_vtx_row dwi_dup_col dwi_dup_row; do
  f=$G/fam/$v.c
  python3 $G/r11/cc.py $f $W/d_$v -dl -dg -df > /dev/null
  for d in lreg greg; do python3 $G/r11/sect.py $W/d_$v/t.i.$d > $W/d_$v/f.$d; done
  python3 $G/fam/gexcerpt.py d_$v
  echo "  .s: $(grep -m1 'sra' d_$v/f.s | tr -s '\t' ' ') | $(grep -m1 'andi.*0x0001' d_$v/f.s | tr -s '\t' ' ')"
done
