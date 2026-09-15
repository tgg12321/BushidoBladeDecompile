#!/bin/bash
# ra_solver.mkasm_honest — produce the asm pair the inverse solver's goal
# derivation needs, for ONE TU, WITHOUT touching src/.
#
#   <stem>.cc1.s   raw cc1 output from the CHEAT-STRIPPED source
#   <stem>.hon.s   + prologue_fix | maspsx | multu_pad
#                  = OURS, the honest pure-C stream
#   <stem>.tgt.s   original source through the same pipeline
#                  = TARGET bytes (the tree builds SHA1-identical, so this
#                    stream IS the original executable's instruction order)
#
# The difference between .hon.s and .tgt.s is exactly the residual the inverse
# solver has to explain.  Deriving it this way is what makes the goal honest:
# for a PARKED function the on-main source carries asm pins that already force
# target's registers, so comparing the on-main build against target yields an
# empty diff and no question to invert.  Stripping first restores the question.
#
# Cheat-stripping reuses engine.inlineasm.write_stripped — the same routine the
# cheat-invisible sandbox uses — so "honest" here means exactly what it means
# everywhere else in the engine.
#
# Usage (WSL, repo root, venv active):  bash tools/ra_solver/mkasm_honest.sh <stem>
set -u
STEM=$1
OUT=tmp/inverse_work
mkdir -p "$OUT/src"

CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC1="tools/gcc-2.7.2/build/cc1"
CC1F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
MASPSX_FLAGS="--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt"

# ---- ours: cheat-stripped source, no rules -------------------------------
python3 -c "
import sys; sys.path.insert(0, '.')
from engine import inlineasm
n = inlineasm.write_stripped('$STEM', '$OUT/src/$STEM.c')
print('  stripped   %d cheat-asm construct(s)' % n)
"
$CPP "$OUT/src/$STEM.c" 2>/dev/null > "$OUT/$STEM.hon.i"
$CC1 $CC1F "$OUT/$STEM.hon.i" -o "$OUT/$STEM.cc1.s" 2>/dev/null
echo "  cc1(hon)   rc=$? lines=$(wc -l < "$OUT/$STEM.cc1.s")"
python3 tools/prologue_fix.py < "$OUT/$STEM.cc1.s" > "$OUT/$STEM.p.s" 2>/dev/null
python3 tools/maspsx/maspsx.py $MASPSX_FLAGS < "$OUT/$STEM.p.s" > "$OUT/$STEM.m.s" 2>/dev/null
python3 tools/multu_pad.py --funcs multu_pad_funcs.txt < "$OUT/$STEM.m.s" > "$OUT/$STEM.hon.s" 2>/dev/null
echo "  honest     rc=$? lines=$(wc -l < "$OUT/$STEM.hon.s")"

# ---- target: original source, all cheat stages applied -------------------
# Owner ruling 2026-08-25 (func_800645B0 packet): a stale .tgt.s left behind a
# failed target half poisons every downstream classifier with fiction — remove
# it up front, and fail LOUDLY below if this half does not produce a fresh one.
# For an INCLUDE_ASM-routed function this half CANNOT produce the target stream
# (src carries no C body since asm-until-matched); use the object-level path:
#   python3 tools/ra_solver/goal_from_tgt.py classify <stem> <func>
rm -f "$OUT/$STEM.tgt.s"
$CPP "src/$STEM.c" 2>/dev/null > "$OUT/$STEM.tgt.i"
$CC1 $CC1F "$OUT/$STEM.tgt.i" -o "$OUT/$STEM.tcc1.s" 2>/dev/null
python3 tools/prologue_fix.py < "$OUT/$STEM.tcc1.s" > "$OUT/$STEM.tp.s" 2>/dev/null
python3 tools/maspsx/maspsx.py $MASPSX_FLAGS < "$OUT/$STEM.tp.s" > "$OUT/$STEM.tm.s" 2>/dev/null
python3 tools/multu_pad.py --funcs multu_pad_funcs.txt < "$OUT/$STEM.tm.s" > "$OUT/$STEM.th.s" 2>/dev/null
cp "$OUT/$STEM.th.s" "$OUT/$STEM.tgt.s"
echo "  target     rc=$? lines=$(wc -l < "$OUT/$STEM.tgt.s")"
if [ ! -s "$OUT/$STEM.tgt.s" ]; then
    rm -f "$OUT/$STEM.tgt.s"
    echo "  TARGET HALF FAILED — no $STEM.tgt.s produced (stale file removed)." >&2
    echo "  For INCLUDE_ASM-routed functions the text path cannot work; use:" >&2
    echo "    python3 tools/ra_solver/goal_from_tgt.py classify $STEM <func>" >&2
fi

rm -f "$OUT/$STEM.p.s" "$OUT/$STEM.m.s" "$OUT/$STEM.tp.s" "$OUT/$STEM.tm.s" \
      "$OUT/$STEM.th.s" "$OUT/$STEM.tcc1.s" "$OUT/$STEM.r1.s" "$OUT/$STEM.r2.s"
