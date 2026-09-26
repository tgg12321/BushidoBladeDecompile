#!/bin/bash
# usage: bash tmp/func_800620B8/s3/dump.sh <variant.c>
# Splice the variant over INCLUDE_ASM in a copy of src/text1b.c, preprocess exactly as the Makefile,
# compile with the instrumented project cc1 (tools/gcc-2.7.2/cc1) and the build's CC_FLAGS plus
# -ds -dL -dt -dl -dg (cse / loop / cse2 / local-alloc / global-alloc dumps), with BB2_ALLOC_DEBUG=1.
# Output: tmp/func_800620B8/s3/d/<name>/{t.i, f.s, *.fn (func_800620B8 section of each dump), alloc.txt, cmd.txt}
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
V=$1
N=$(basename $V .c)
D=tmp/func_800620B8/s3/d/$N
mkdir -p $D
rm -f $D/t.* $D/*.fn
python3 - "$V" "$D/src.c" <<'PY'
import sys
v=open(sys.argv[1]).read()
s=open('src/text1b.c').read()
k='INCLUDE_ASM("asm/funcs", func_800620B8);'
assert k in s
open(sys.argv[2],'w').write(s.replace(k,v))
PY
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC1="tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
$CPP $D/src.c > $D/t.i 2>/dev/null
{ echo "cpp: $CPP src.c > t.i"; echo "cc1: BB2_ALLOC_DEBUG=1 $CC1 -ds -dL -dt -dl -dg t.i -o t.s"; } > $D/cmd.txt
cd $D
BB2_ALLOC_DEBUG=1 ../../../../../$CC1 -ds -dL -dt -dl -dg t.i -o t.s 2> alloc_all.txt
grep "func=func_800620B8" alloc_all.txt > alloc.txt || true
rm -f alloc_all.txt
for f in t.i.*; do
python3 - "$f" <<'PY'
import sys,re
p=sys.argv[1]
t=open(p,errors='replace').read()
m=re.search(r'\n;; Function func_800620B8\n', t)
if m:
    rest=t[m.start():]
    n=re.search(r'\n;; Function (?!func_800620B8)', rest[10:])
    open(p.replace('t.i.','')+'.fn','w').write(rest[: n.start()+10] if n else rest)
PY
rm -f $f
done
python3 - <<'PY'
import re
t=open('t.s').read()
m=re.search(r'\nfunc_800620B8:\n(.*?)\n\t\.end\tfunc_800620B8', t, re.S)
open('f.s','w').write(m.group(1))
PY
rm -f t.s
ls
