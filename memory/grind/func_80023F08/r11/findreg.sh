#!/bin/bash
# findreg.sh <tag> <pseudo>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80023F08/dumps/$1"
BB2_FINDREG_DEBUG=$2 ../../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/null tu.i 2>&1 | grep -A9 "func=func_80023F08 pseudo=$2 " | head -24
