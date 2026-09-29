#!/bin/bash
# Ruling 11 (D)(1) dumps for func_8002A458: reuse spelling vs one-variable-per-value spellings.
# stock = build compiler tools/gcc-2.7.2/build/cc1; dbg = instrumented tools/gcc-2.7.2/cc1
# (BB2_ALLOC_DEBUG priority/seat trace). Each dbg .s is checked identical to the stock .s.
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
for v in final onevar_full dxyz_all_own temp_all_own temp2_split; do
  python3 tmp/func_8002A458/mk.py tmp/func_8002A458/r11v/$v.c a458_${v}_stock >/dev/null
  python3 tmp/func_8002A458/mk.py tmp/func_8002A458/r11v/$v.c a458_${v}_dbg CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 >/dev/null
  if cmp -s tmp/rtl/a458_${v}_stock/func.s tmp/rtl/a458_${v}_dbg/func.s; then echo "$v: stock==dbg asm"; else echo "$v: stock!=dbg asm"; fi
done
