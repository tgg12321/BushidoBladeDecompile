#!/bin/bash
# usage: alloc9.sh <name>... (needs tmp/c21c/out/<name>.i from orph.py)
# 1) project cc1 -dl -dg -df dumps; 2) BB2_ALLOC_DEBUG global.c allocation order from the
# instrumented diagnostic cc1 (tools/gcc-2.7.2/cc1), whose .s is checked identical to build cc1.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
mkdir -p tmp/c21c9/dumps
for n in "$@"; do
  echo "== $n"
  echo "cmd: tools/gcc-2.7.2/build/cc1 $F -dl -dg -df tmp/c21c/out/$n.i"
  tools/gcc-2.7.2/build/cc1 $F -dl -dg -df tmp/c21c/out/$n.i -o /tmp/c21c9_$n.s
  for p in lreg greg flow; do
    awk '/^;; Function func_8006C21C/{p=1} /^;; Function /&&!/func_8006C21C/{p=0} p' tmp/c21c/out/$n.i.$p > tmp/c21c9/dumps/$n.$p
  done
  echo "cmd: BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $F tmp/c21c/out/$n.i"
  BB2_ALLOC_DEBUG=1 tools/gcc-2.7.2/cc1 $F tmp/c21c/out/$n.i -o /tmp/c21c9_dbg_$n.s 2> tmp/c21c9/dumps/$n.allocdbg.all
  cmp -s /tmp/c21c9_dbg_$n.s /tmp/c21c9_$n.s && echo "diag cc1 .s identical to build cc1" || echo "DIAG OUTPUT DIFFERS"
  grep "func=func_8006C21C" tmp/c21c9/dumps/$n.allocdbg.all > tmp/c21c9/dumps/$n.allocdbg || true
  wc -l < tmp/c21c9/dumps/$n.allocdbg
done
