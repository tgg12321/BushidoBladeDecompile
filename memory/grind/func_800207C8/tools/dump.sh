#!/bin/bash
# usage: dump.sh <tag>  -- RTL dumps of func_800207C8 from tmp/func_800207C8/w/<tag>/code6cac_tu2.c (run h.py first)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tmp/func_800207C8/w/$1
mipsel-linux-gnu-cpp -I$D -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/code6cac_tu2.c > $D/tu.i 2>/dev/null
cd $D
rm -f tu.i.*
../../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dL -dg -dl -ds -dc -dj tu.i -o tu.s 2>/dev/null || true
for f in tu.i.*; do python3 - "$f" <<'PY'
import sys
p=sys.argv[1]; t=open(p).read()
i=t.find('\n;; Function func_800207C8')
if i<0: sys.exit()
j=t.find('\n;; Function ', i+10)
open(p+'.fn','w').write(t[i:j if j>0 else len(t)])
PY
done
ls *.fn
