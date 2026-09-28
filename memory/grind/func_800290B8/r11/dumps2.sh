#!/bin/bash
# FINDREGDBG for the counter pseudo (82 in both spellings) + flow register stats.
cd "$(dirname "$0")/../.." || exit 1
source .venv/bin/activate 2>/dev/null
for v in final split; do
  python3 tmp/f290b8/mk.py tmp/f290b8/$v.c b8_${v}_fr82 CC1=tools/gcc-2.7.2/cc1 BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=82 >/dev/null
  echo "== $v"
  grep -A6 "FINDREGDBG func=func_800290B8" tmp/rtl/b8_${v}_fr82/stderr.txt
  grep -E "^Register (82|83|84|85) " tmp/rtl/b8_${v}_stock/f.flow
done
