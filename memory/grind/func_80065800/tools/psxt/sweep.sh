#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
while read c; do
  mipsel-linux-gnu-cpp -undef -lang-c -Dmips -D__GNUC__=2 "$c" > "$c.i" 2>/dev/null
  tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$c.i" -o "$c.s" 2>/dev/null || { echo "FAIL $c"; continue; }
  python3 tmp/f65800/psxt/chk.py "$c.s"
done < tmp/f65800/psxt/sw.lst
