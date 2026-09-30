#!/bin/bash
# dump.sh <cand.c> <tag> [cc1 dump flags]: splice candidate into text1b_tu1c.c copy, compile with RTL dumps
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
C=$1; T=$2; shift 2
FL=${@:--dS -dl -dg -dR -dc}
D=tmp/func_8006CFBC/dump_$T
mkdir -p $D
python3 - "$C" "$D" <<'PY'
import sys
c, d = sys.argv[1], sys.argv[2]
src = open('src/text1b_tu1c.c').read()
cand = open(c).read()
key = 'INCLUDE_ASM("asm/funcs", func_8006CFBC);'
assert key in src
open(d + '/tu.c', 'w').write(src.replace(key, cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/tu.c > $D/f.i 2>/dev/null
tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $FL $D/f.i -o $D/f.s
# cut the function out of each dump
for f in $D/f.i.*; do
  python3 - "$f" <<'PY'
import sys, re
p = sys.argv[1]
t = open(p, errors='replace').read()
m = re.search(r'\n;; Function func_8006CFBC\n(.*?)(?=\n;; Function |\Z)', t, re.S)
open(p + '.fn', 'w').write(m.group(1) if m else '')
PY
done
awk '/^func_8006CFBC:/,/\.end\tfunc_8006CFBC/' $D/f.s > $D/fn.s
echo done $D
