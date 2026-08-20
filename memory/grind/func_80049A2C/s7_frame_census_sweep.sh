#!/bin/bash
S=tmp/grind/func_80049A2C/s7
OUT=$S/sweep
mkdir -p $OUT
CPP_FLAGS="-Iinclude -undef -Wall -lang-c -fno-builtin"
CPP_DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx"
for f in src/*.c; do
  b=$(basename $f .c)
  mipsel-linux-gnu-cpp $CPP_FLAGS $CPP_DEFS $f 2>/dev/null > $OUT/$b.i
  tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel < $OUT/$b.i > $OUT/$b.s 2>/dev/null
  echo "$b $(grep -c '\.frame' $OUT/$b.s)"
done
