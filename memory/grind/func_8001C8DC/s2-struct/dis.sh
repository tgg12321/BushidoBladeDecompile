#!/bin/bash
# dis.sh <start_hex> <end_hex>: disassemble scratch EXE vs original over a vaddr range
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
S=$(( 0x$1 - 0x80010000 + 0x800 )); N=$(( 0x$2 - 0x$1 ))
for f in tmp/c8dc2/tree/build/bb2.exe disc/SLUS_006.63; do
  dd if=$f of=/tmp/c8dc_$$.bin bs=1 skip=$S count=$N 2>/dev/null
  mipsel-linux-gnu-objdump -D -b binary -m mips --adjust-vma=0x$1 /tmp/c8dc_$$.bin | tail -n +8 | cut -f3- > /tmp/c8dc_$$_$(basename $f).txt
done
diff /tmp/c8dc_$$_bb2.exe.txt /tmp/c8dc_$$_SLUS_006.63.txt
