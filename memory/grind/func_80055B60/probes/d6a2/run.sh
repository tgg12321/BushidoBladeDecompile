#!/bin/bash
# run.sh <candidate.c>: build text1b with the candidate + tmp header, score every function vs build/src/text1b.o
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
python3 tmp/func_80055B60/hdr.py tmp/func_80055B60/inc/include/code6cac.h tmp/func_80055B60/d6a/include/code6cac.h >/dev/null || exit 1
python3 tmp/func_80055B60/mksrc.py "${1:-tmp/func_80055B60/cand1.c}" tmp/func_80055B60/text1b.c >/dev/null || exit 1
python3 memory/grind/func_80055B60/probes/prep_d6a78/tucheck.py text1b tmp/func_80055B60/text1b.c tmp/func_80055B60/inc/include 2>&1 | tail -15
