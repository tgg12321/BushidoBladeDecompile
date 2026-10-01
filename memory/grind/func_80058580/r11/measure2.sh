#!/bin/bash
# measure2.sh <files...>: each body spliced with memory/grind/func_80058580/typed/* via typed/run.sh; prints differing words + masked score
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=memory/grind/func_80058580/typed
for f in "$@"; do
  n=$(basename $f .c); d=tmp/func_80058580/r11b/o/$n; mkdir -p $d
  cp $f $d/f58580.c; cp $T/src_edits.py $T/hdr_edits.py $d/
  out=$(bash tmp/func_80058580/h/run.sh $d 2>&1 | grep -E "^text:|func scores|parse error" | sed 's/func scores nonzero: //' | tr '\n' ' ')
  ins=$(python3 tmp/func_80058580/h/fd.py $d 2>&1 | tail -1)
  echo "$n | $out | $ins"
done
