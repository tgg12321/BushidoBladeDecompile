#!/bin/bash
# compile.sh <in.c> -o <out.o>  (build recipe for src/text1b_tu1c.c, -G0 file)
set -e
IN="$1"; OUT="$3"
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < "$IN" 2>/dev/null \
 | python3 tools/prologue_fix.py \
 | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
 | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
 | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
