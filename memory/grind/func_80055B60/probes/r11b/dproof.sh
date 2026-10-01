#!/bin/bash
# dproof.sh: Ruling 11 (D)(1) record for the landing body -> tmp/func_80055B60/r11/d_proof.txt
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
R=tmp/func_80055B60/r11
bash $R/dump.sh landing >/dev/null; bash $R/dump.sh pv >/dev/null
O=$R/d_proof.txt
{
echo "# Ruling 11 (D)(1) record for func_80055B60 (laneC 2026-10-01), on the exact landing body."
echo "# Bodies: landing.c (= candidate.c, reuse spelling) and pv.c (one variable per value, mkpv.py)."
echo "# Command: dump.sh <name> (text1b with the body spliced by mksrc.py; cpp with the landing header ahead of include/ |"
echo "#   tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da),"
echo "#   then tools/gcc-2.7.2/cc1 (instrumented) with BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo> on the same tu.i (findreg.sh)."
echo
for spec in "landing:79:work 80:temp 81:temp2 82:temp3 83:i" "pv:79:diff 484:sign 81:dist 602:n 592:add 723:mask 788:da 82:ret 80:lim 656:near 761:len 591:least 657:far 83:i"; do
  name=${spec%%:*}; rest=${spec#*:}
  echo "=== $name"
  for pv in $rest; do
    p=${pv%%:*}; label=${pv#*:}
    echo "--- pseudo $p ($label)"
    grep "^Register $p used" $R/d_$name/f.lreg
    bash $R/findreg.sh $name $p | grep -v "used_so_far\|used2\|class"
  done
done
} > $O
echo done
