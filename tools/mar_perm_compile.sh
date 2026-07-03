#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"
OUT="$3"
TMPS=$(mktemp /tmp/marXXXXXX.s)
trap 'rm -f "$TMPS" "$TMPS.fn.s"' EXIT
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
    -mips1 -mno-abicalls -fno-builtin -w "$IN" -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt \
      --sdata-exclude=sdata_exclude.txt --expand-lb \
      --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt \
      --expand-dest-funcs=expand_dest_funcs.txt \
      --label-nop-funcs=maspsx_label_nop_funcs.txt \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
awk 'BEGIN{printf "\t.text\n\t.align\t2\n"}
/^\.globl\tmarionation_Exec$/ {p=1}
p {print}
p && /^\.end\tmarionation_Exec$/ {exit}
' "$TMPS" > "$TMPS.fn.s"
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 \
    -no-pad-sections -O1 -G0 -o "$OUT" "$TMPS.fn.s"
