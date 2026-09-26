#!/bin/bash
# Calibration only: original PsyQ cc1psx on a scratch tree's TU (preprocessed with that tree's include/).
# usage: psx.sh <tree> <stem> <outdir> [G-flags...]  -> <outdir>/<stem>.psx<G>.s and ours<G>.s
set -o pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=tmp/func_80036140/$1; S=$2; O=tmp/func_80036140/psx/$3; shift 3
mkdir -p $O
mipsel-linux-gnu-cpp -I$T/include -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $T/src/$S.c > $O/$S.i 2>/dev/null
for G in "${@:--G8}"; do
  bash tools/cc1psx_wrapper.sh -O2 $G -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w < $O/$S.i > $O/$S.psx$G.s 2>$O/$S.psx$G.err || echo "cc1psx $G rc=$?"
  tools/gcc-2.7.2/build/cc1 -O2 $G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < $O/$S.i > $O/$S.ours$G.s 2>/dev/null
  echo "$G: psx $(wc -l < $O/$S.psx$G.s) lines, ours $(wc -l < $O/$S.ours$G.s) lines"
done
echo "cc1psx: bash tools/cc1psx_wrapper.sh -O2 <G> -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w < $S.i" > $O/CMD
