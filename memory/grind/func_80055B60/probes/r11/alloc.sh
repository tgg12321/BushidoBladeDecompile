#!/bin/bash
# alloc.sh <name>: instrumented cc1 BB2_ALLOC_DEBUG on the spliced TU (from dump.sh's tu.i), function only
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/b60/d_$1"
BB2_ALLOC_DEBUG=1 ../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float tu.i -o /dev/null 2> alloc.all
awk '/func_80055B60/{p=1} p' alloc.all | head -5 >/dev/null
