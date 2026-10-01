#!/bin/bash
# dproof.sh: Ruling 11 (D)(1) dumps for func_80055B60 -> tmp/b60/d_proof.txt
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
O=tmp/b60/d_proof.txt
{
echo "# Ruling 11 (D)(1) record for func_80055B60 (laneA 2026-10-01)."
echo "# Bodies: fin.c (= the landing body, reuse spelling) and pvf.c (one variable per value, made by mkpv.py)."
echo "# Command: tmp/b60/dump.sh <name> (cpp with the PracticeMenuRec header of mkhdr.py ahead of include/ |"
echo "#   tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da),"
echo "#   then tools/gcc-2.7.2/cc1 (instrumented) with BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo> on the same tu.i."
echo
for spec in "fin:79:work 80:temp 81:temp2 82:temp3 83:i" "pvf:79:diff 484:sign 81:dist 602:n 592:add 722:mask 787:da 82:ret 80:lim 655:near 760:len 591:least 656:far 83:i"; do
  name=${spec%%:*}; rest=${spec#*:}
  echo "=== $name"
  grep "Register dispositions" -A30 tmp/b60/d_$name/f.greg | head -0
  for pv in $rest; do
    p=${pv%%:*}; label=${pv#*:}
    echo "--- pseudo $p ($label)"
    grep "^Register $p used" tmp/b60/d_$name/f.lreg
    bash tmp/b60/findreg.sh $name $p | grep -v "used_so_far\|used2\|class"
  done
done
} > $O
echo done
