#!/bin/bash
# Apply candidate, run instrumented cc1 with QTYDBG, print marionation's
# do_timeout-block qty allocations (block-isolated via the name stream).
set -e
cd "$(dirname "$0")/.."
cp src/system.c tmp/system.c.bak
python3 tmp/mar_apply_candidate.py > /dev/null
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips \
  -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
  src/system.c > tmp/rtl/marQ.i 2>/dev/null
BB2_QTY_DEBUG=1 tmp/gccdbg/cc1 -O2 -G0 -funsigned-char -mcpu=3000 \
  -mips1 -mno-abicalls -fno-builtin -w -da tmp/rtl/marQ.i -o /dev/null \
  2> tmp/rtl/marQ.log || true
cp tmp/system.c.bak src/system.c
sed 's/ marionation_Exec/\nMARKSTART\n/; s/ tslTm2LoadImage/\nMARKEND\n/' \
  tmp/rtl/marQ.log \
  | awk '/MARKSTART/{m=1;next} /MARKEND/{m=0} m && /QTYDBG/' \
  | grep "blk=3 "
