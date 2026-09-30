#!/bin/bash
# Allocator dumps for the sanctioned-family probe fam/dw_init.c (the per-value body with the marker
# index's init wrapped in do { } while (0)), same cc1 flags as r11/dump.sh (-dl -dg -df added).
# Usage: bash memory/grind/func_800290B8/fam/dump_fam.sh  (from the repo root)
set -e
R=/home/user/BushidoBladeDecompile
G=$R/memory/grind/func_800290B8
W=$R/tmp/w290; mkdir -p $W
cd $W
for v in dw_init; do
  python3 $G/r11/cc.py $G/fam/$v.c $W/d_$v -dl -dg -df > /dev/null
  for d in lreg greg flow; do python3 $G/r11/sect.py $W/d_$v/t.i.$d > $W/d_$v/f.$d; done
done
python3 $G/r11/excerpt.py dw_init
echo "=== dw_init: the bounding-box loop's i/2 and i&1 insns, .lreg"
grep -B1 -A2 "(ashiftrt:SI\|(and:SI (reg/v:SI 80)" $W/d_dw_init/f.lreg | head -12
echo "=== dw_init: block-local pseudos seated by local-alloc (';; Register N in R.')"
grep "^;; Register [0-9]* in " $W/d_dw_init/f.lreg | head -20
echo "=== dw_init: .s lines of the bounding-box index math and the marker index"
grep -n "sra\|andi\|\$8" $W/d_dw_init/f.s | head -20
