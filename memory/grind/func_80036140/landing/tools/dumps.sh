#!/bin/bash
# RTL dumps (our cc1, the build's flags) of code6cac_b5.c for a tree + optional body variant.
# usage: dumps.sh <tree> <outdir> <G8|G0> [variant.c]
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
T=tmp/func_80036140/$1; O=tmp/func_80036140/dumps/$2; G=$3; V=$4
mkdir -p $O; rm -f $O/*
if [ -n "$V" ]; then bash tmp/func_80036140/vb.sh $1 $V $G >/dev/null; fi
cp $T/src/code6cac_b5.c $O/b5.c
mipsel-linux-gnu-cpp -I$T/include -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $O/b5.c > $O/b5.i 2>/dev/null
( cd $O && ../../../../tools/gcc-2.7.2/build/cc1 b5.i -O2 -$G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -ds -dt -dc -dl -dg -o b5.s )
echo "cmd: cc1 b5.i -O2 -$G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -ds -dt -dc -dl -dg" > $O/CMD
ls $O
