#!/bin/bash
# chk.sh: score every function of the modified code6cac_b_tu2 / code6cac_b TUs (tmp copies + tmp header) vs build/src
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
D=tmp/func_80055B60/d6a
python3 memory/grind/func_80055B60/probes/prep_d6a78/tucheck.py code6cac_b_tu2 $D/src/code6cac_b_tu2.c $D/include 2>&1 | tail -${1:-12}
python3 memory/grind/func_80055B60/probes/prep_d6a78/tucheck.py code6cac_b $D/src/code6cac_b.c $D/include 2>&1 | tail -3
python3 memory/grind/func_80055B60/probes/prep_d6a78/tucheck.py code6cac_tu2 $D/src/code6cac_tu2.c $D/include 2>&1 | tail -3
