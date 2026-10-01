#!/bin/bash
# dumpall.sh: dumps for the landing body and its one-variable-per-value twins (header overlay =
# tmp/func_8002AB08/typed/inc, i.e. the PracticeMenuRec members this landing adds; see typed/hdr.py)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=tmp/func_8002AB08/r11
export HDR_I="-Itmp/func_8002AB08/typed/inc/include"
bash $R/dump.sh cand tmp/func_8002AB08/typed/final.c
for v in dx dy dz temp1 temp2 temp3 idx alt work all; do bash $R/dump.sh pv_$v $R/fpv_$v.c; done
