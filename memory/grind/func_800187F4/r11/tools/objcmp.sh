#!/bin/bash
# objcmp.sh <tag>: compare every allocated section of tmp/func_800187F4/fast_<tag>/f.o with build/src/code6cac.o
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
A=tmp/func_800187F4/fast_$1/f.o
B=build/src/code6cac.o
for s in .text .rodata .data .sdata .bss; do
  if ! mipsel-linux-gnu-objdump -h $B | grep -q " $s "; then continue; fi
  mipsel-linux-gnu-objcopy -O binary --only-section=$s $A /tmp/objcmp_a.bin
  mipsel-linux-gnu-objcopy -O binary --only-section=$s $B /tmp/objcmp_b.bin
  if cmp -s /tmp/objcmp_a.bin /tmp/objcmp_b.bin; then echo "$s identical ($(stat -c %s /tmp/objcmp_a.bin) bytes)"; else echo "$s DIFFERS"; cmp -l /tmp/objcmp_a.bin /tmp/objcmp_b.bin | head -5; fi
done
diff <(mipsel-linux-gnu-objdump -r $A | sed 1,3d) <(mipsel-linux-gnu-objdump -r $B | sed 1,3d) > /tmp/objcmp_rel.diff && echo "relocations identical" || { echo "relocations differ: $(grep -c '^[<>]' /tmp/objcmp_rel.diff) lines"; head -10 /tmp/objcmp_rel.diff; }
