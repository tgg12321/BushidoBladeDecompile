#!/bin/bash
# ev_obj.sh TU ALIAS=BASE+OFF... : evidence for a relocation-only merge diff (base snapshot vs build/)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
t=$1; shift
A=tmp/p2/snap/base/obj/$t.o; B=build/src/$t.o
for s in .text .data .rodata .sdata .bss .sbss; do
  a=$(mipsel-linux-gnu-objcopy -O binary -j $s $A /dev/stdout 2>/dev/null | sha1sum | cut -c1-12)
  b=$(mipsel-linux-gnu-objcopy -O binary -j $s $B /dev/stdout 2>/dev/null | sha1sum | cut -c1-12)
  echo "$t $s $a -> $b"
done
echo "--- instruction words that differ"
diff <(mipsel-linux-gnu-objdump -d $A | grep -P '^\s+[0-9a-f]+:') <(mipsel-linux-gnu-objdump -d $B | grep -P '^\s+[0-9a-f]+:')
echo "--- relocations (symbol index dropped)"
diff <(mipsel-linux-gnu-readelf -rW $A | awk '/R_MIPS/{print $1, $3, $5}') <(mipsel-linux-gnu-readelf -rW $B | awk '/R_MIPS/{print $1, $3, $5}')
echo "--- folded disassembly"
python3 tmp/p2/relnorm.py $A $B "$@"
