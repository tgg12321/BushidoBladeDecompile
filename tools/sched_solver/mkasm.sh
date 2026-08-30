#!/bin/bash
# Produce, for one TU, the three aligned asm texts the goal-mapper needs:
#   <stem>.cc1.s   raw cc1 output              (index-aligns 1:1 with .dbr UIDs)
#   <stem>.hon.s   + prologue_fix|maspsx|multu_pad  (= OURS, honest)
#   <stem>.tgt.s   same pipeline (rule stages retired 2026-08-30)     (= TARGET bytes)
# Run from a snapshot root.  Usage: bash mkasm.sh <stem>
set -u
STEM=$1
OUT=tmp/sched_map
mkdir -p "$OUT"

CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC1="tools/gcc-2.7.2/build/cc1"
CC1F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel"
MASPSX_FLAGS="--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --label-nop-funcs=maspsx_label_nop_funcs.txt"

$CPP "src/$STEM.c" 2>/dev/null > "$OUT/$STEM.i"
$CC1 $CC1F "$OUT/$STEM.i" -o "$OUT/$STEM.cc1.s" 2>/dev/null
echo "  cc1        rc=$? lines=$(wc -l < $OUT/$STEM.cc1.s)"

python3 tools/prologue_fix.py < "$OUT/$STEM.cc1.s" > "$OUT/$STEM.p.s" 2>/dev/null
echo "  prologue   rc=$? lines=$(wc -l < $OUT/$STEM.p.s)"

python3 tools/maspsx/maspsx.py $MASPSX_FLAGS < "$OUT/$STEM.p.s" > "$OUT/$STEM.m.s" 2>/dev/null
echo "  maspsx     rc=$? lines=$(wc -l < $OUT/$STEM.m.s)"

python3 tools/multu_pad.py --funcs multu_pad_funcs.txt < "$OUT/$STEM.m.s" > "$OUT/$STEM.hon.s" 2>/dev/null
echo "  multu_pad  rc=$? lines=$(wc -l < $OUT/$STEM.hon.s)"

# Owner ruling 2026-08-25 (func_800645B0 packet): never leave a stale .tgt.s
# behind a failed target half — it poisons goal derivation with fiction. For
# INCLUDE_ASM-routed functions this stream cannot carry the target; use the
# object-level goal path (--target-object build/src/<stem>.o).
rm -f "$OUT/$STEM.tgt.s"
cp "$OUT/$STEM.hon.s" "$OUT/$STEM.tgt.s"
echo "  cheats     rc=$? lines=$(wc -l < $OUT/$STEM.tgt.s)"
if [ ! -s "$OUT/$STEM.tgt.s" ]; then
    rm -f "$OUT/$STEM.tgt.s"
    echo "  TARGET HALF FAILED — no $STEM.tgt.s produced (stale file removed)." >&2
fi
rm -f "$OUT/$STEM.p.s" "$OUT/$STEM.m.s" "$OUT/$STEM.r1.s" "$OUT/$STEM.r2.s" "$OUT/$STEM.i"
