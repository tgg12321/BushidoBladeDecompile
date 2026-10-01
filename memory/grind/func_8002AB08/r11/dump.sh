#!/bin/bash
# dump.sh <tag> <body.c>: RTL/allocation dumps of func_8002AB08 for one body spliced over its INCLUDE_ASM in
# src/code6cac_b_tu2.c (build cc1 -dr -dl -dg, then the instrumented cc1 with BB2_ALLOC_DEBUG=1; asm compared)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
tag=$1; body=$2
D=tmp/func_8002AB08/r11/dumps/$tag; rm -rf $D; mkdir -p $D
python3 - "$body" "$D/tu.c" <<'PY'
import sys
import subprocess
src = subprocess.run(['git', 'show', 'HEAD:src/code6cac_b_tu2.c'], capture_output=True, text=True).stdout
line = 'INCLUDE_ASM("asm/funcs", func_8002AB08);'
assert src.count(line) == 1
open(sys.argv[2], 'w').write(src.replace(line, open(sys.argv[1]).read()))
PY
mipsel-linux-gnu-cpp ${HDR_I} -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/tu.c > $D/t.i 2>/dev/null
python3 tools/decomp-permuter/strip_other_fns.py $D/t.i func_8002AB08
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
( cd $D && ../../../../../tools/gcc-2.7.2/build/cc1 $FLAGS -dr -dl -dg t.i -o t.s )
( cd $D && BB2_ALLOC_DEBUG=1 ../../../../../tools/gcc-2.7.2/cc1 $FLAGS t.i -o t.inst.s 2> t.alloc.raw )
grep "func=func_8002AB08" $D/t.alloc.raw | grep ALLOCDBG > $D/$tag.alloc || true
for f in rtl lreg greg; do
  awk '/^;; Function func_8002AB08/{p=1} /^;; Function /&&!/func_8002AB08/{p=0} p' $D/t.i.$f > $D/$tag.$f
done
grep "^Register " $D/$tag.lreg > $D/$tag.regs || true
awk '/^func_8002AB08:/{p=1} p{print} /\.end\tfunc_8002AB08/{p=0}' $D/t.s > $D/$tag.fn.s
awk '/^func_8002AB08:/{p=1} p{print} /\.end\tfunc_8002AB08/{p=0}' $D/t.inst.s > $D/$tag.inst.fn.s
if cmp -s $D/$tag.fn.s $D/$tag.inst.fn.s; then echo "$tag IDENTITY OK"; else echo "$tag IDENTITY MISMATCH"; fi
rm -f $D/t.i.rtl $D/t.i.lreg $D/t.i.greg $D/t.inst.s $D/t.s $D/tu.c
