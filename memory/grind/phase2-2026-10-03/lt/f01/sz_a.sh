#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mipsel-linux-gnu-cpp -Iinclude -undef -D__GNUC__=2 -Dmips -D__mips__ -D_LANGUAGE_C -DLANGUAGE_C memory/grind/phase2-2026-10-03/lt/f01/sz_a.c > tmp/sz_a.i 2>&1
tools/gcc-2.7.2/build/cc1 -O2 -quiet -mel -msoft-float tmp/sz_a.i -o /dev/null 2>&1 | grep -v warning | head
echo "cc1 rc=${PIPESTATUS[0]}"
