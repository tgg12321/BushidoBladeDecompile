#!/bin/bash
# find_reg trace (BB2_FINDREG_DEBUG) for the site-1 island-input pseudo (89) in the reuse and split spellings.
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
for v in final temp2_split; do
  python3 tmp/func_8002A458/mk.py tmp/func_8002A458/r11v/$v.c a458_${v}_fr89 CC1=tools/gcc-2.7.2/cc1 BB2_FINDREG_DEBUG=89 >/dev/null
  echo "== $v"
  grep -A8 "FINDREGDBG func=func_8002A458" tmp/rtl/a458_${v}_fr89/stderr.txt
  grep "pseudo=89 " tmp/rtl/a458_${v}_dbg/stderr.txt | grep 8002A458
  grep "^Register 89 " tmp/rtl/a458_${v}_stock/f.lreg
done
