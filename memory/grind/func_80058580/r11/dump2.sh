#!/bin/bash
# dump2.sh <tag> <body.c>: RTL/allocation dumps of func_80058580 for one body spliced with memory/grind/func_80058580/typed/*
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
tag=$1; body=$2; T=memory/grind/func_80058580/typed
D=tmp/func_80058580/r11b/dumps/$tag; rm -rf $D; mkdir -p $D/v
cp $body $D/v/f58580.c; cp $T/f55138.c $T/f56fe8.c $T/src_edits.py $T/hdr_edits.py $D/v/
python3 tmp/func_80058580/h/splice.py $D/v $D/s > /dev/null
mipsel-linux-gnu-cpp -I$D/s/inc -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/s/text1b.c > $D/t.i
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
