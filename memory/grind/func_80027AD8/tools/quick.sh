#!/bin/bash
# usage: quick.sh <variant.c>... : compile each with cc1 and print prologue signature
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
out=tmp/func_80027AD8/d_q; mkdir -p $out
for v in "$@"; do
python3 - "$v" "$out" <<'PY'
import sys
cand=open(sys.argv[1]).read()
src=open('src/code6cac_b.c').read()
line='INCLUDE_ASM("asm/funcs", func_80027AD8);'
src=src.replace('INCLUDE_RODATA("asm/rodata", jtbl_80010548);\n','').replace('extern s32 func_80027AD8(s32, u8 *, s32, s32, s32, Tbl8008E194 *, s32, s32 *);','')
open(sys.argv[2]+'/x.c','w').write(src.replace(line,cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $out/x.c 2>/dev/null > $out/x.i
(cd $out && ../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float x.i -o x.s 2>/dev/null)
python3 tmp/func_80027AD8/sig.py $out/x.s
echo "^ $(basename $v)"
done
