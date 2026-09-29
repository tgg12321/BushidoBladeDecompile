#!/bin/bash
# dump.sh <tag> [cc1 dump flags] : splice out_<tag>.c into code6cac.c and compile with RTL dumps
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
T=${1:-x}; shift
FL=${@:--dL -dg -dl -df -dc -dj -ds -dt -dS -dR}
D=tmp/func_800187F4/dump_$T
mkdir -p $D
python3 - "$T" <<'PY'
import sys
t=sys.argv[1]
src=open('src/code6cac.c').read()
cand=open(f'tmp/func_800187F4/out_{t}.c').read()
key='INCLUDE_ASM("asm/funcs", func_800187F4);'
assert key in src
src=src.replace('void func_800187F4(s32 arg0, s32 *arg1);','void func_800187F4(s16 *arg0, s32 *arg1);') if 'func_800187F4(s16 *arg0' in cand else src
open(f'tmp/func_800187F4/dump_{t}/code6cac.c','w').write(src.replace(key,cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/code6cac.c > $D/f.i 2>/dev/null
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $FL $D/f.i -o $D/f.s
