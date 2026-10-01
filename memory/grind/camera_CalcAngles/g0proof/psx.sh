#!/bin/bash
# psx.sh <file.c> [G]: calibration only - original cc1psx on a probe at -G$G
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
g=${2:-0}
bash tools/cc1psx_wrapper.sh -O2 -G$g -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < "$1" | grep -vE '^\s*#|^$|^\s*\.(frame|mask|fmask|file|ent|end|text|align|globl|set|lcomm)'
