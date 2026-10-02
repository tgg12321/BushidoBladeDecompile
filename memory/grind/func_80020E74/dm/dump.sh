#!/bin/bash
# usage: dump.sh <candidate.c> <outtag>  -- substitutes candidate into a copy of code6cac_tu2.c and dumps RTL for func_80020E74
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tmp/func_80020E74/rtl/$2
mkdir -p $D
python3 -c "
import sys; sys.path.insert(0,'.')
from engine import inlineasm
from pathlib import Path
b=Path('src/code6cac_tu2.c').read_text(); c=Path('$1').read_text()
Path('$D/tu.c').write_text(inlineasm.substitute_body(b,'func_80020E74',c))
"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/tu.c > $D/tu.i 2>/dev/null
cd $D
../../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dg -dl -dL -dj -dc -ds tu.i -o tu.s 2>/dev/null || true
for f in tu.i.*; do python3 - "$f" <<'PY'
import sys,re
p=sys.argv[1]; t=open(p).read()
i=t.find('\n;; Function func_80020E74')
if i<0: sys.exit()
j=t.find('\n;; Function ', i+10)
open(p+'.fn','w').write(t[i:j if j>0 else len(t)])
PY
done
ls
