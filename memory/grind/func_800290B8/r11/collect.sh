#!/bin/bash
# Assemble r11/dumps.txt from tmp/rtl/b8_* (run dumps.sh + dumps2.sh first).
# Variable -> pseudo mapping is DERIVED from each spelling's own .lreg by pmap.py (writes
# classified as const / self-increment / i & 1 / i >> 1), never assumed.
cd "$(dirname "$0")/../.." || exit 1
R=tmp/rtl; P=tmp/f290b8/pmap.py
for v in final split; do
  echo "################ $v spelling (tmp/f290b8/$v.c)"
  echo "## commands"; cat $R/b8_${v}_stock/cmd.txt; cat $R/b8_${v}_fr82/cmd.txt
  echo "   (second line: instrumented cc1, env BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=82)"
  echo "## user-variable pseudos from f.lreg (pmap.py), with f.flow stats and f.lreg local-alloc dispositions"
  python3 $P $R/b8_${v}_stock
  echo "## f.greg ';; Register dispositions' entries for these pseudos (absent = no hard reg)"
  if [ $v = final ]; then PS="82 83"; else PS="82 93 94 176"; fi
  for p in $PS; do grep -oE "(^| )$p in -?[0-9]+" $R/b8_${v}_stock/f.greg | sed "s/^ //" | sed "s/^/   /"; done
  echo "## the i & 1 and i >> 1 insns (f.lreg)"
  grep -B1 -A1 -E "\(and:SI \(reg/v:SI 80\)" $R/b8_${v}_stock/f.lreg | head -3
  grep -B1 -A1 -E "\(ashiftrt:SI \(reg:SI" $R/b8_${v}_stock/f.lreg | head -3
  echo "## ALLOCDBG (global_alloc order) for the list-index / triangle-number pseudos"
  grep "func=func_800290B8" $R/b8_${v}_fr82/stderr.txt | grep -E "pseudo=(82|83|176) "
  echo "## FINDREGDBG pseudo 82 (acc=0 = first pass; acc=1 = caller-save retry)"
  grep -A6 "FINDREGDBG func=func_800290B8" $R/b8_${v}_fr82/stderr.txt
  echo "## asm: head-loop sra/andi and the list index's seat"
  grep -nE "andi|sra|24\(.sp\)|move	.8,.0" $R/b8_${v}_stock/func.s | head -12
  echo
done
