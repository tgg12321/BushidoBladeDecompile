#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
S=memory/grind/func_80058580/r11/findreg.sh
bash $S cand 84 85 86 87 88 > tmp/func_80058580/r11/dumps/findreg_cand.txt
bash $S pv_work5 90 92 > tmp/func_80058580/r11/dumps/findreg_pv_work5.txt
bash $S pv_work1 94 97 > tmp/func_80058580/r11/dumps/findreg_pv_work1.txt
bash $S pv_work2 96 > tmp/func_80058580/r11/dumps/findreg_pv_work2.txt
bash $S pv_work3 92 101 > tmp/func_80058580/r11/dumps/findreg_pv_work3.txt
bash $S pv_work4 90 92 > tmp/func_80058580/r11/dumps/findreg_pv_work4.txt
