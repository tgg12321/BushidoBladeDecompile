#!/bin/bash
# runall.sh : every (D)(1) dump for the submission, then the excerpt file.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/func_8001F2E4
bash $T/tools/dumpall.sh $T/r11v $T/dumps R_base:78,79,80,81,82 split_all:78,79,80,81,188,214,215 \
  only_V1:82,83 only_V2:82,83 only_V3 only_V4 only_V5 only_V6 only_V56 only_W2:78 only_X2 only_Z2 only_XZ2
bash $T/tools/dumpall.sh $T/r11t $T/dumps T1_literal0 T2_init_once:79 T3_after_join T4_before_calls
python3 $T/tools/collect.py $T/dumps $T/dumps.txt
