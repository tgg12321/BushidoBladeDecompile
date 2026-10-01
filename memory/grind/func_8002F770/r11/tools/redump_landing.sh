#!/bin/bash
# laneB's r11 dump tooling on the landing chassis (fresh i0/i1) and its per-value spellings.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=memory/grind/func_8002F770/r11/tools
V=memory/grind/func_8002F770/r11/variants_landing
F=func_8002F770
run() { bash $T/dumps.sh $F "$1" "tmp/F770c/$2" > /dev/null && python3 $T/cut.py "tmp/F770c/$2" $F > /dev/null; }
run $V/reuse.c d_reuse
run $V/pv_both.c d_pv
run $V/pv_work_only.c d_split
run $V/pv_temp_only.c d_sumsplit
python3 $T/r11table.py tmp/F770c/d_reuse:$F tmp/F770c/d_pv:$F tmp/F770c/d_split:$F tmp/F770c/d_sumsplit:$F
python3 $T/shiftseat.py tmp/F770c/d_reuse:$F tmp/F770c/d_pv:$F tmp/F770c/d_sumsplit:$F
