#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mipsel-linux-gnu-cpp -Itmp/p2/wk/include -Iinclude -undef -D__GNUC__=2 -Dmips -D__mips__ -D_LANGUAGE_C -DLANGUAGE_C tmp/p2/lt/f02/t/sz2.c > tmp/p2/lt/f02/t/sz2.i 2>&1
tools/gcc-2.7.2/build/cc1 -O2 -quiet -mel -msoft-float tmp/p2/lt/f02/t/sz2.i -o /dev/null 2>&1 | grep -v warning | head
echo "cc1 rc=${PIPESTATUS[0]}"
