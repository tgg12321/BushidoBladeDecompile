#!/bin/bash
# fdiff.sh <func> [obj]: side-by-side objdump of one function, scratch vs build/src/text1b.o
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
f=$1; o=${2:-tmp/func_80057E84/dm/text1b.o}
mipsel-linux-gnu-objdump -d --no-show-raw-insn --disassemble=$f $o | grep -E '^ +[0-9a-f]+:' | sed 's/^ *[0-9a-f]*:\t//' > /tmp/fa.txt
mipsel-linux-gnu-objdump -d --no-show-raw-insn --disassemble=$f build/src/text1b.o | grep -E '^ +[0-9a-f]+:' | sed 's/^ *[0-9a-f]*:\t//' > /tmp/fb.txt
diff /tmp/fb.txt /tmp/fa.txt
