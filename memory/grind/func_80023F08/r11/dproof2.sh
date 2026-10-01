#!/bin/bash
# Ruling 11 (D)(1) record for func_80023F08's temp on the landing body (src/code6cac_tu2.c as staged) vs the
# one-variable-per-value spelling (r11b/split_all.c substituted with engine.inlineasm.substitute_body).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
D=tmp/func_80023F08/dumps2; rm -rf $D; mkdir -p $D/final $D/split_all
cp src/code6cac_tu2.c $D/final/tu.c
python3 -c "
import sys; sys.path.insert(0,'.')
from engine import inlineasm
t=open('src/code6cac_tu2.c').read(); b=open('tmp/func_80023F08/r11b/split_all.c').read()
open('$D/split_all/tu.c','w').write(inlineasm.substitute_body(t,'func_80023F08',b))"
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for t in final split_all; do
  $CPP $D/$t/tu.c > $D/$t/tu.i 2>/dev/null
  (cd $D/$t && ../../../../tools/gcc-2.7.2/build/cc1 $FLAGS -dl -dg -o tu.s tu.i)
  awk '/^;; Function func_80023F08$/{f=1} f&&/^;; Function /&&!/func_80023F08$/{f=0} f' $D/$t/tu.i.lreg > $D/$t/f.lreg
done
echo "# Ruling 11 (D)(1) record for func_80023F08 temp (laneC 2026-10-01), measured on the landing body."
echo "# final = src/code6cac_tu2.c as staged for the Match; split_all = r11/r11gen.py one-variable-per-value spelling."
echo "# cpp ($CPP) | tools/gcc-2.7.2/build/cc1 $FLAGS -dl -dg; then tools/gcc-2.7.2/cc1 (instrumented, identical .s)"
echo "# with BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo> on the same tu.i."
for spec in final:81:temp split_all:81:lim split_all:82:gap split_all:83:turn split_all:84:side; do
  t=${spec%%:*}; r=${spec#*:}; p=${r%%:*}; n=${r#*:}
  echo; echo "=== $t pseudo $p ($n)"
  grep "^Register $p used" $D/$t/f.lreg
  grep "^;; Register $p in" $D/$t/f.lreg && echo "(local-alloc: block-local quantity)"
  (cd $D/$t && BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$p ../../../../tools/gcc-2.7.2/cc1 $FLAGS -o /dev/null tu.i 2>&1 | grep -A8 "FINDREGDBG func=func_80023F08 pseudo=$p " | grep -v "used_so_far\|used2_noconflict\|class="; BB2_ALLOC_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 $FLAGS -o /dev/null tu.i 2>&1 | grep "ALLOCDBG func=func_80023F08 .*pseudo=$p ")
done
