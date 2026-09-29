#!/bin/bash
# FINDREG traces: final pseudo 80 (temp) and 251 (smode); split 250 (sr_rate), 306 (sl_rate), and split's smode.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" || exit 1
source .venv/bin/activate 2>/dev/null
for spec in final:80 final:251 split:250 split:306 split:${SPLIT_SMODE:-252}; do
  v=${spec%%:*}; p=${spec##*:}
  python3 tmp/f8b488s4/mk.py tmp/f8b488s4/$v.c b488_${v}_fr$p CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$p >/dev/null
  cmp -s tmp/rtl/b488_${v}_stock/func.s tmp/rtl/b488_${v}_fr$p/func.s && echo "$v fr$p: stock==dbg asm" || echo "$v fr$p: stock!=dbg asm"
done
