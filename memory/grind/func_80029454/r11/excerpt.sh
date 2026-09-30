#!/bin/bash
# Excerpts of the allocation dumps for the Ruling 11 (D) record of `rec`.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/func_80029454/r11"
for v in reuse split; do
  echo "############ $v body ############"
  echo "--- .flow (func_80029454): record-pointer pseudos"
  if [ $v = reuse ]; then P="80"; else P="100|265"; fi
  grep -E "^Register ($P) used" d_$v/f.flow
  echo "--- .lreg: the two record-pointer sets (insn 121 = first record loop, insn 615/616 = second)"
  grep -E "^\(insn (121|615|616) " -A2 d_$v/f.lreg
  echo "--- .greg: the same insns after global allocation"
  grep -E "^\(insn (121|615|616) " -A2 d_$v/f.greg
  echo "--- .greg Register dispositions (pseudo in hardreg; 17 = s1, 6 = a2)"
  grep -A60 "Register dispositions" d_$v/f.greg | tr '\t' ' ' | grep -oE "\b($P) in [0-9]+"
done
