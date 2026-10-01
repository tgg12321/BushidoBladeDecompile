#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash memory/grind/func_80058580/r11/dump.sh cand memory/grind/func_80058580/candidate.c
for w in work1 work2 work3 work4 work5 all; do
  bash memory/grind/func_80058580/r11/dump.sh pv_$w tmp/func_80058580/r11/v2/pv_$w.c
done
