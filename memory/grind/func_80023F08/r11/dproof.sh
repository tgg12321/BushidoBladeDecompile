#!/bin/bash
# Ruling 11 (D)(1) record for func_80023F08's temp: reuse (final = landing body) vs one variable per value (split_all).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80023F08/dumps"
CC1="../../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o /dev/null"
echo "# Ruling 11 (D)(1) record for func_80023F08 temp (laneC 2026-10-01)."
echo "# Bodies: final = body5 (landing body, reuse); split_all = one variable per value (r11gen.py)."
echo "# Dumps: dump.sh (cpp with the landing include/code6cac.h | tools/gcc-2.7.2/build/cc1 <build flags> -ds -dj -dt -dl -dg);"
echo "# then tools/gcc-2.7.2/cc1 (instrumented, identical .s) with BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo> on the same tu.i."
for spec in final:81:temp split_all:81:lim split_all:82:gap split_all:83:turn split_all:84:side; do
  t=${spec%%:*}; r=${spec#*:}; p=${r%%:*}; n=${r#*:}
  echo; echo "=== $t pseudo $p ($n)"
  grep "^Register $p used" $t/f.lreg
  grep "^;; Register $p in" $t/f.lreg && echo "(local-alloc: block-local quantity)"
  (cd $t && BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$p $CC1 tu.i 2>&1 | grep -A8 "FINDREGDBG func=func_80023F08 pseudo=$p " | grep -v "used_so_far\|used2_noconflict\|class=" ; BB2_ALLOC_DEBUG=1 $CC1 tu.i 2>&1 | grep "ALLOCDBG func=func_80023F08 .*pseudo=$p ")
done
