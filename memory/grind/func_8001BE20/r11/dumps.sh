#!/bin/bash
# Ruling 11 (D)(1) dumps for func_8001BE20: reuse vs one-variable-per-value spelling.
# stock = the build compiler (tools/gcc-2.7.2/build/cc1); dbg = instrumented cc1 with alloc debug hooks.
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
for v in reuse split; do
  python3 tmp/f1be20s4/mk.py tmp/f1be20s4/$v.c be20_${v}_stock
  python3 tmp/f1be20s4/mk.py tmp/f1be20s4/$v.c be20_${v}_dbg CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=1
  echo "== $v: move a3,a2 count (stock / dbg)"
  grep -c "move	\$7,\$6" tmp/rtl/be20_${v}_stock/func.s
  grep -c "move	\$7,\$6" tmp/rtl/be20_${v}_dbg/func.s
  cmp -s tmp/rtl/be20_${v}_stock/func.s tmp/rtl/be20_${v}_dbg/func.s && echo "stock==dbg asm" || echo "stock!=dbg asm"
done
