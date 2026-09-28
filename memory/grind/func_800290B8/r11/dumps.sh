#!/bin/bash
# Ruling 11 (D)(1) dumps for func_800290B8: reuse (final.c) vs one-variable-per-value (split.c).
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
for v in final split; do
  python3 tmp/f290b8/mk.py tmp/f290b8/$v.c b8_${v}_stock
  python3 tmp/f290b8/mk.py tmp/f290b8/$v.c b8_${v}_dbg CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=1
  cmp -s tmp/rtl/b8_${v}_stock/func.s tmp/rtl/b8_${v}_dbg/func.s && echo "$v: stock==dbg asm" || echo "$v: stock!=dbg asm"
done
