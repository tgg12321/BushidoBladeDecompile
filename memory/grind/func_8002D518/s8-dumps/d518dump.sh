#!/bin/bash
# cse / greg dumps + final .s of func_8002D518, landed form (A) vs single-write control (B).
# usage (WSL, repo root, after python3 tmp/laneH/d518dump.py): bash tmp/laneH/d518dump.sh <outdir>
set -e
OUT=$1; mkdir -p $OUT
CC1=tools/gcc-2.7.2/cc1
DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for v in A B; do
  mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $DEFS tmp/laneH/d518_$v/src/code6cac_b_tu2.c > /tmp/d518_$v.i 2>/dev/null
  $CC1 $FLAGS -ds /tmp/d518_$v.i -o /tmp/d518_$v.s
  # the func_8002D518 section of the .cse dump, and its register-to-register copies
  awk '/^;; Function func_8002D518/{p=1;next} p&&/^;; Function /{exit} p' /tmp/d518_$v.i.cse > $OUT/$v.cse.txt
  grep -n "(set (reg[^)]*) (reg[^)]*))" $OUT/$v.cse.txt > $OUT/$v.copies.txt || true
  awk '/^func_8002D518:/,/\.end\tfunc_8002D518/' /tmp/d518_$v.s > $OUT/$v.s.txt
  echo "$v: $(wc -l < $OUT/$v.copies.txt) reg-reg copies in .cse; $(grep -c '^\s[a-z]' $OUT/$v.s.txt) asm lines"
done
echo "cmd: mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $DEFS <tree>/src/code6cac_b_tu2.c | $CC1 $FLAGS -ds" > $OUT/commands.txt
