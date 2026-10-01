#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=tmp/func_80058580/r11b
for w in reuse pv_work1 pv_work2 pv_work3 pv_work4 pv_work5 pv_all; do
  bash $R/dump2.sh $w $R/b/$w.c
done
