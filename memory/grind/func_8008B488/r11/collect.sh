#!/bin/bash
# Assemble r11/dumps.txt from tmp/rtl/b488_* (run dumps.sh, dumps2.sh with SPLIT_SMODE=251, dumpesc.sh first).
# Variable -> pseudo mapping is DERIVED from each spelling's own f.lreg by pmap.py.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
R=tmp/rtl
for v in final split; do
  echo "################ $v spelling (tmp/f8b488s4/$v.c)"
  echo "## commands (build cc1; then the instrumented cc1 with env BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=<pseudo>)"
  cat $R/b488_${v}_stock/cmd.txt; cat $R/b488_${v}_dbg/cmd.txt
  cmp -s $R/b488_${v}_stock/func.s $R/b488_${v}_dbg/func.s && echo "identity: build cc1 asm == instrumented cc1 asm for func_8008B488"
  echo "## ADSR-value pseudos (pmap.py: writes classified from f.lreg; f.flow stats; dispositions; ALLOCDBG)"
  python3 tmp/f8b488s4/pmap.py $R/b488_${v}_stock $R/b488_${v}_dbg
  echo "## smode (pseudo 251: the SI pseudo set to 0x100 / 0 / 0x200 / 0x300 in the SR block)"
  grep -E "^Register 251 " $R/b488_${v}_stock/f.flow
  grep -E "pseudo=251 " $R/b488_${v}_dbg/stderr.txt | grep func_8008B488
  echo "## global_alloc order (ALLOCDBG, func_8008B488, ord 0-19)"
  grep "func=func_8008B488 ord=" $R/b488_${v}_dbg/stderr.txt | awk -F'ord=' '{split($2,a," "); if (a[1] < 20) print}'
  if [ $v = final ]; then PS="80 251"; else PS="250 251 306"; fi
  for p in $PS; do
    echo "## FINDREGDBG pseudo $p"
    grep -A10 "FINDREGDBG func=func_8008B488" $R/b488_${v}_fr$p/stderr.txt
  done
  echo "## asm (build cc1, before maspsx): SR block rate/smode seats and the SL block"
  grep -nE "lhu	\\\$[0-9]+,5[26]\(\\\$16\)|li	\\\$[0-9]+,0x0000007f|li	\\\$[0-9]+,0x0000000f|li	\\\$[0-9]+,0x00000[123]00|or	\\\$2,\\\$[0-9]+,\\\$[0-9]+|andi	\\\$4,\\\$2,0x003f" $R/b488_${v}_stock/func.s | tail -14
  echo
done
echo "################ escape variants (split.c + one sanctioned construct; tmp/f8b488s4/fam*/)"
grep -vE "used_so_far|pass0_used|pass1_used|used2_noconflict" tmp/f8b488s4/dumpesc.log
echo "################ fam3 variants (split.c + a FAKE write and a chain-extender read of sl_rate in another block)"
grep -vE "used_so_far|pass0_used|pass1_used|used2_noconflict" tmp/f8b488s4/dumpfam3.log
for t in f3load; do echo "## $t: insns mentioning (reg/v:HI 80) in f.flow vs f.combine"; for p in flow combine; do echo -n "$p: "; python3 -c "import re,sys; t=open(\"tmp/rtl/b488_${t}_stock/f.$p\").read(); b=re.split(r\"\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )\",t); print([re.match(r\"\((\w+) (\d+)\",x).group(2) for x in b if re.match(r\"\((\w+) (\d+)\",x) and \"reg/v:HI 80\" in x])"; done; done
echo "## f3load: blocks whose live-at-start set contains pseudo 80 (f.flow)"
grep -B2 -E "Registers live at start:.*( 80( |$))" tmp/rtl/b488_f3load_stock/f.flow
echo "## f3load: the combine-left USE of pseudo 80 in the SR block"
grep -A2 "^(insn 1139 " tmp/rtl/b488_f3load_stock/f.combine
echo "################ fam4 variants (split.c + a FAKE write and a chain-extender read of sl_rate in DIFFERENT basic blocks)"
grep -vE "used_so_far|pass0_used|pass1_used|used2_noconflict" tmp/f8b488s4/dumpfam4.log
echo "## f4pos: blocks whose live-at-start set contains pseudo 80 (f.flow)"
grep -B2 -E "Registers live at start:.*( 80( |$))" tmp/rtl/b488_f4pos_stock/f.flow | grep "Basic block"
echo "## f4pos: start of the SR store block after combine (the death USE 1147 precedes the adsr mask 843)"
awk '/^\(code_label 828 /{p=1} p&&/^\(insn 843 /{print; exit} p' tmp/rtl/b488_f4pos_stock/f.combine
