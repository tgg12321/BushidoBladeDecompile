#!/bin/bash
# dump.sh <tag> <body.c>: RTL/allocation dumps of func_80058580 for one body (r11 proof)
#  1. splice the body into a scratch text1b.c with the landing's declaration edits (probes/s5/splice.py)
#  2. cpp (build CPP flags, patched include first) and strip the other function bodies
#  3. build cc1 (tools/gcc-2.7.2/build/cc1) -dr -dl -dg  -> <tag>.rtl / .lreg / .greg / .s
#  4. instrumented cc1 (tools/gcc-2.7.2/cc1) with BB2_ALLOC_DEBUG=1 -> <tag>.alloc (+ identity check of the asm)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
tag=$1; body=$2
D=tmp/func_80058580/r11/dumps/$tag; rm -rf $D; mkdir -p $D
python3 memory/grind/func_80058580/probes/s5/splice.py $body $D/t.c
mipsel-linux-gnu-cpp -Itmp/func_80058580/inc -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/t.c > $D/t.i
python3 tools/decomp-permuter/strip_other_fns.py $D/t.i func_80058580
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
( cd $D && ../../../../../tools/gcc-2.7.2/build/cc1 $FLAGS -dr -dl -dg t.i -o t.s )
( cd $D && BB2_ALLOC_DEBUG=1 ../../../../../tools/gcc-2.7.2/cc1 $FLAGS t.i -o t.inst.s 2> t.alloc.raw )
grep "func=func_80058580" $D/t.alloc.raw | grep ALLOCDBG > $D/$tag.alloc || true
for f in rtl lreg greg; do
  awk '/^;; Function func_80058580/{p=1} /^;; Function /&&!/func_80058580/{p=0} p' $D/t.i.$f > $D/$tag.$f
done
awk '/^func_80058580:/{p=1} p{print} /\.end\tfunc_80058580/{p=0}' $D/t.s > $D/$tag.fn.s
awk '/^func_80058580:/{p=1} p{print} /\.end\tfunc_80058580/{p=0}' $D/t.inst.s > $D/$tag.inst.fn.s
if cmp -s $D/$tag.fn.s $D/$tag.inst.fn.s; then echo "$tag IDENTITY OK"; else echo "$tag IDENTITY MISMATCH"; fi
rm -f $D/t.i.rtl $D/t.i.lreg $D/t.i.greg $D/t.inst.s $D/t.s
