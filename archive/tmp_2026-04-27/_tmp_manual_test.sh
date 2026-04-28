#!/bin/bash
set -o pipefail
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
source .venv/bin/activate 2>/dev/null

mipsel-linux-gnu-cpp \
  -I"$ROOT"/include -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips \
  -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ \
  -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C -DPERMUTER \
  "$ROOT"/_tmp_func_8003F6D8.c \
  | "$ROOT"/tools/gcc-2.7.2/build/cc1 \
    -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w \
  | python3 "$ROOT"/tools/maspsx/maspsx.py \
    --expand-div --aspsx-version=2.34 \
    --sdata-syms="$ROOT"/sdata_syms.txt \
    --sdata-funcs="$ROOT"/sdata_funcs.txt \
    --sdata-exclude="$ROOT"/sdata_exclude.txt \
  | mipsel-linux-gnu-as \
    -I"$ROOT"/include -march=r3000 -mtune=r3000 \
    -no-pad-sections -O1 -G0 -o "$ROOT"/_tmp_func_8003F6D8.o

echo "EXIT CODE: $?"
