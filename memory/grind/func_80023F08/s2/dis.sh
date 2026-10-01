#!/bin/bash
# disassemble func_80023F08 from sandbox object and target object
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
mipsel-linux-gnu-objdump -d --no-show-raw-insn tmp/sandbox/func_80023F08/code6cac_tu2.o | awk '/<func_80023F08>:/{f=1} f&&/^$/{if(c++)exit} f' > tmp/func_80023F08/ours.s
mipsel-linux-gnu-objdump -d --no-show-raw-insn build/src/code6cac_tu2.o | awk '/<func_80023F08>:/{f=1} f&&/^$/{if(c++)exit} f' > tmp/func_80023F08/target.s
wc -l tmp/func_80023F08/ours.s tmp/func_80023F08/target.s
