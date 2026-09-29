#!/bin/bash
# Ruling 11 (D)(1) dumps for func_8008B488: reuse (final.c) vs one-variable-per-value (split.c).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate 2>/dev/null
for v in final split; do
  python3 tmp/f8b488s4/mk.py tmp/f8b488s4/$v.c b488_${v}_stock
  python3 tmp/f8b488s4/mk.py tmp/f8b488s4/$v.c b488_${v}_dbg CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=${FR:-1}
  cmp -s tmp/rtl/b488_${v}_stock/func.s tmp/rtl/b488_${v}_dbg/func.s && echo "$v: stock==dbg asm" || echo "$v: stock!=dbg asm"
done
