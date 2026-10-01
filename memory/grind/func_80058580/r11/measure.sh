#!/bin/bash
# measure.sh <files...>: for each candidate body: full-pipeline text1b.o, exact link check, masked score
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for f in "$@"; do
  n=$(basename $f .c)
  out=$(OUTD=tmp/func_80058580/r11/o_$n bash memory/grind/func_80058580/probes/s5/fullobj.sh $f 2>&1 | grep -E "^text:|^score|rror" | tr '\n' ' ')
  echo "$n | $out"
done
