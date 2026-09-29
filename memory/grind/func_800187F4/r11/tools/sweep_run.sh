#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate 2>/dev/null
python3 tmp/f187/mkfast4.py
python3 tmp/f187/sweep_prep.py
TAGS=$(cut -f1 tmp/f187/sweep_tags.tsv)
: > tmp/f187/sweep.log
for T in $TAGS; do
  bash tmp/func_800187F4/fast4.sh $T >> tmp/f187/sweep.log 2>&1
  printf "FRAME %s %s\n" $T "$(grep -m1 -o 'addiu sp,sp,-[0-9]*' tmp/func_800187F4/fast_$T/ours.s 2>/dev/null)" >> tmp/f187/sweep.log
done
# joined-mode measurement of the pure-C attempt [2]
for T in $(grep -P '\tr11/banked/pc_joined.c$' tmp/f187/sweep_tags.tsv | cut -f1); do
  echo "JOINED:" >> tmp/f187/sweep.log
  GENMODE=--joined bash tmp/func_800187F4/fast4.sh $T >> tmp/f187/sweep.log 2>&1
done
echo SWEEP-DONE >> tmp/f187/sweep.log
