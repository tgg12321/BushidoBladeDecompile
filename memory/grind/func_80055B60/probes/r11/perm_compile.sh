#!/bin/bash
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
IN="$1"
OUT="$3"
TMPS=$(mktemp /tmp/pb60_XXXXXX.s)
trap "rm -f $TMPS $TMPS.pre $TMPS.fn.s $TMPS.pl.s" EXIT
mipsel-linux-gnu-cpp -Itmp/b60/inc/include -Iinclude -undef -Wall -lang-c -fno-builtin \
    -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
    -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
    "$IN" 2>/dev/null \
  | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 \
        -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/stdout 2>/dev/null \
  | python3 tools/prologue_fix.py \
  | grep -v $'^\t\.file\t' > "$TMPS.pre"
python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 \
      --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt \
      --sdata-exclude=sdata_exclude.txt --expand-lb \
      --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt \
      --expand-dest-funcs=expand_dest_funcs.txt \
      --prefill-label-funcs=maspsx_prefill_label_funcs.txt \
      < "$TMPS.pre" \
  | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > "$TMPS"
awk '
/\.ent[ \t]+func_80055B60$/ {p=1}
/glabel func_80055B60$/ {p=1}
p {print}
p && /\.end[ \t]+func_80055B60$/ {exit}
' "$TMPS" > "$TMPS.fn.s"
printf '.set noat\n.set noreorder\n' > "$TMPS.pl.s"
cat "$TMPS.pl.s" "$TMPS.fn.s" \
  | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 \
      -no-pad-sections -O1 -G0 -o "$OUT"
