#!/bin/bash
# dump.sh <body.c> <tag> : cc1 -dl -dg dumps for code6cac_tu2 with the body -> tmp/func_80023F08/dumps/<tag>/
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
d=tmp/func_80023F08/dumps/$2; mkdir -p $d
python3 tmp/func_80023F08/tu_edit.py $1 $d/src >/dev/null
mipsel-linux-gnu-cpp -Itmp/func_80023F08/inc -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $d/src/code6cac_tu2.c > $d/tu.i 2>/dev/null
(cd $d && ../../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -ds -dj -dt -dl -dg -o tu.s tu.i)
ls $d
