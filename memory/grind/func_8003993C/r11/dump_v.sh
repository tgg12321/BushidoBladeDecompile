#!/bin/bash
# dump candidate.c (tag cand) and every r11/v/<x>.c (tag v_<x>)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
bash tmp/func_8003993C/r11/dump.sh cand memory/grind/func_8003993C/candidate.c
for f in tmp/func_8003993C/r11/v/*.c; do b=$(basename $f .c); bash tmp/func_8003993C/r11/dump.sh v_$b $f; done
