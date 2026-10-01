#!/bin/bash
# cc1out.sh <tu>: pre-maspsx stream (cpp|cc1|prologue_fix) of the model tree's TU to /tmp/q56/cc1_<tu>.s
cd /tmp/q56/model
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C src/$1.c 2>/dev/null | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float | python3 tools/prologue_fix.py > /tmp/q56/cc1_$1.s
