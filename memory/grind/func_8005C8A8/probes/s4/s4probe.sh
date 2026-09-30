#!/bin/bash
# s4probe.sh <body.c>...: build each body in a copy of src/text1b_tu1c.c against a copy of include/
# with the fix1 game.h hunk applied (tracked files untouched); print frame + objdump diff lines vs target.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
W=tmp/func_8005C8A8/s4w
mkdir -p $W
if [ ! -d $W/include ]; then
  cp -r include $W/include
  python3 tmp/func_8005C8A8/s4hdr.py $W/include/game.h
fi
{ echo '.include "include/macro.inc"'; echo '.set noat'; echo '.set noreorder'; echo '.section .text'; cat asm/funcs/func_8005C8A8.s; } > $W/target.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $W/target.o $W/target.s
f() { mipsel-linux-gnu-objdump -dr "$1" | awk '/<func_8005C8A8>:/{p=1;next} /^[0-9a-f]+ </{p=0} p' | sed 's/^ *[0-9a-f]*:\t[0-9a-f ]*\t//' | grep -v '^$'; }
f $W/target.o > $W/t.txt
for B in "$@"; do
  n=$(basename $B .c)
  python3 tmp/func_8005C8A8/s4splice.py "$B" "$W/$n.c"
  mipsel-linux-gnu-cpp -I$W/include -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $W/$n.c 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float 2>$W/$n.err > $W/$n.s
  fr=$(grep -A2 "^func_8005C8A8:" $W/$n.s | grep frame | tr -s '\t ' ' ')
  python3 tools/prologue_fix.py < $W/$n.s | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
   | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $W/$n.o 2>>$W/$n.err
  f $W/$n.o > $W/$n.txt
  echo "$n: $fr | diff lines $(diff $W/t.txt $W/$n.txt | grep -c '^[<>]') | insns $(wc -l < $W/$n.txt)/$(wc -l < $W/t.txt)"
done
