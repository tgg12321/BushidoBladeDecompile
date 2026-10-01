#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=tmp/func_8002AB08/r11
bash $R/dump.sh cand memory/grind/func_8002AB08/candidate.c
for v in dx dy dz temp1 temp2 temp3 idx alt work all; do bash $R/dump.sh pv_$v $R/pv_$v.c; done
