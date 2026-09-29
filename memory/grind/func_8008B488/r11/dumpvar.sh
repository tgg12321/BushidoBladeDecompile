#!/bin/bash
# usage: dumpvar.sh <variant.c> <tag>  -> stock + instrumented dumps, pseudo map, and a
# FINDREG trace for every ADSR-value pseudo (tmp/rtl/b488_<tag>_*)
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate 2>/dev/null
V=$1; T=$2
python3 tmp/f8b488s4/mk.py $V b488_${T}_stock >/dev/null
python3 tmp/f8b488s4/mk.py $V b488_${T}_dbg CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 >/dev/null
cmp -s tmp/rtl/b488_${T}_stock/func.s tmp/rtl/b488_${T}_dbg/func.s && echo "[$T] stock==dbg asm" || echo "[$T] stock!=dbg asm"
python3 tmp/f8b488s4/pmap.py tmp/rtl/b488_${T}_stock tmp/rtl/b488_${T}_dbg | tee tmp/rtl/b488_${T}_stock/pmap.txt
for p in $(grep -oE '^pseudo [0-9]+' tmp/rtl/b488_${T}_stock/pmap.txt | awk '{print $2}'); do
  python3 tmp/f8b488s4/mk.py $V b488_${T}_fr$p CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$p >/dev/null
  echo "  FINDREG pseudo $p:"; grep -A7 "FINDREGDBG func=func_8008B488" tmp/rtl/b488_${T}_fr$p/stderr.txt | sed 's/^/    /'
  grep -E "^Register $p " tmp/rtl/b488_${T}_stock/f.flow | sed 's/^/    flow: /'
done
