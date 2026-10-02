#!/bin/bash
# usage: dis.sh <func> [obj]  -> side-by-side diff of ref build/src/text1b.o vs obj
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
o=${2:-tmp/func_8005490C/xb/text1b.o}
ref=${3:-build/src/text1b.o}
mipsel-linux-gnu-objdump -d -r --disassemble=$1 $ref | sed -n '/<'$1'>:/,$p' | sed 's/^ *[0-9a-f]*:\t[0-9a-f ]*\t//' > /tmp/r.txt
mipsel-linux-gnu-objdump -d -r --disassemble=$1 $o | sed -n '/<'$1'>:/,$p' | sed 's/^ *[0-9a-f]*:\t[0-9a-f ]*\t//' > /tmp/o.txt
diff /tmp/r.txt /tmp/o.txt
