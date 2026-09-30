#!/bin/bash
# RTL dumps (the build cc1, the b5 recipe's -G8 flags) of a scratch variant tree v/<var>/ and bank excerpts
# of the E9C/EA4 sites. usage: dumps_q62.sh <var> <label> <pattern>
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
V=tmp/func_80036140/v/$1; O=tmp/func_80036140/dq62/$2; PAT=$3
mkdir -p $O; rm -f $O/*
cp $V/code6cac_b5.c $O/b5.c
mipsel-linux-gnu-cpp -I$V/inc -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $O/b5.c > $O/b5.i 2>/dev/null
FLAGS="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -ds -dt -dc -dl -dg"
( cd $O && ../../../../tools/gcc-2.7.2/build/cc1 b5.i $FLAGS -o b5.s )
echo "cmd: tools/gcc-2.7.2/build/cc1 b5.i $FLAGS (header dir tmp/func_80036140/v/$1/inc)" > $O/CMD
python3 memory/grind/func_80036140/landing/tools/excerpt.py $O b5 func_80036140 $O/excerpt.txt 4 "${@:3}"
sed -i "2i cmd: tools/gcc-2.7.2/build/cc1 b5.i $FLAGS; variant $1 (tmp/func_80036140/mk.py)" $O/excerpt.txt
wc -l $O/excerpt.txt
