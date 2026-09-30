#!/bin/bash
# od.sh ours.o [ref.o] : diff func_80036140 disassembly (with relocs) ours vs ref
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
REF=${2:-build/src/code6cac_b5.o}
F=${3:-func_80036140}
mipsel-linux-gnu-objdump -dr --no-show-raw-insn --disassemble=$F $1 | sed -n '/<'$F'>:/,$p' | sed 's/^ *[0-9a-f]*://' > /tmp/od_ours.txt
mipsel-linux-gnu-objdump -dr --no-show-raw-insn --disassemble=$F $REF | sed -n '/<'$F'>:/,$p' | sed 's/^ *[0-9a-f]*://' > /tmp/od_ref.txt
diff /tmp/od_ref.txt /tmp/od_ours.txt
echo "diff rc=$?"
