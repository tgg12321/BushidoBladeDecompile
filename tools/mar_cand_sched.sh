#!/bin/bash
# Apply candidate, run instrumented cc1 with SCHEDDBG, save the log.
set -e
cd "$(dirname "$0")/.."
cp src/system.c tmp/system.c.bak
python3 tmp/mar_apply_candidate.py > /dev/null
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips \
  -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C \
  src/system.c > tmp/rtl/marS.i 2>/dev/null
BB2_SCHED_DEBUG=1 tmp/gccdbg/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 \
  -mips1 -mno-abicalls -fno-builtin -w -da tmp/rtl/marS.i -o /dev/null \
  2> tmp/rtl/marS.sched.log || true
cp tmp/system.c.bak src/system.c
echo "log lines: $(wc -l < tmp/rtl/marS.sched.log)"
