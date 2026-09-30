#!/bin/bash
# usage: run.sh <c file>  -> <c>.ours.s and <c>.psx.s
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
c="$1"
mipsel-linux-gnu-cpp -undef -lang-c -Dmips -D__GNUC__=2 "$c" > "$c.i"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$c.i" -o "$c.ours.s"
bash tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w -msoft-float < "$c.i" > "$c.psx.s"
echo ok
