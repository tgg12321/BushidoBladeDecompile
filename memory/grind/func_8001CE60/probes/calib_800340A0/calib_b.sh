#!/bin/bash
# usage: calib_b.sh <variant.c> <name>: splice func_800340A0 variant into code6cac_b.c, compile with build cc1 and cc1psx
set -e
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
v=$1; n=$2; o=tmp/func_8001CE60/out_b; mkdir -p $o
python3 - "$v" "$o/$n.full.c" <<'PY'
import sys
src=open('src/code6cac_b.c').read()
i=src.index("void func_800340A0(void) {"); j=src.index("void func_80034200(void) {")
open(sys.argv[2],'w',newline='\n').write(src[:i]+open(sys.argv[1]).read()+src[j:])
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $o/$n.full.c 2>/dev/null > $o/$n.i || true
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $o/$n.i -o $o/$n.s
bash tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w -msoft-float < $o/$n.i | tr -d '\r' > $o/$n.psx.s
for f in $o/$n.s $o/$n.psx.s; do awk '/^func_800340A0:/{p=1} p{print} p&&/\.end\tfunc_800340A0/{exit}' $f | grep -v "^\s*#" > $f.fn; done
echo "$n cc1: $(grep -c 'la\t' $o/$n.s.fn) la  | cc1psx: $(grep -c 'la\t' $o/$n.psx.s.fn) la"
grep -m3 "D_800A3898\|g_match_p1_score" $o/$n.s.fn | tr '\n' ';'; echo; grep -m3 "D_800A3898\|g_match_p1_score" $o/$n.psx.s.fn | tr '\n' ';'; echo
