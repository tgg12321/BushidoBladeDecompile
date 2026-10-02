#!/bin/bash
# usage: dumpf.sh <tu.c path (repo-relative)> <func> <outdir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=$3; mkdir -p $D
SRCDIR=$(dirname $1)
mipsel-linux-gnu-cpp -I$SRCDIR -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $1 > $D/tu.i 2>/dev/null
cd $D
"/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tools/gcc-2.7.2/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dg -dl -dL -dj -dc -ds tu.i -o tu.s 2>/dev/null || true
for f in tu.i.*; do python3 - "$f" "$2" <<'PY'
import sys
p,fn=sys.argv[1],sys.argv[2]; t=open(p).read()
i=t.find('\n;; Function '+fn+'\n')
if i<0: sys.exit()
j=t.find('\n;; Function ', i+10)
open(p+'.'+fn,'w').write(t[i:j if j>0 else len(t)])
PY
done
