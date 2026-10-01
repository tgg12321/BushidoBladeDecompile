#!/bin/bash
# a458check.sh : func_8002A458 with `u32 *hit, u32 *deep` vs as committed: compare its cc1 output
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=tmp/func_8002AB08/typed
python3 - <<'PY'
s = open('src/code6cac_b_tu2.c').read()
a = 'void func_8002A458(u8 *obj, s32 *hit, s32 *deep, s32 quiet) {'
assert s.count(a) == 1
open('tmp/func_8002AB08/typed/a458_new.c', 'w').write(s.replace(a, 'void func_8002A458(u8 *obj, u32 *hit, u32 *deep, s32 quiet) {'))
PY
CPP="mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC="tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for f in src/code6cac_b_tu2.c $D/a458_new.c; do $CPP $f 2>/dev/null | $CC -o $D/x.s; awk '/^func_8002A458:/,/\.end\tfunc_8002A458/' $D/x.s > $D/$(basename $f).a458.s; done
cmp -s $D/code6cac_b_tu2.c.a458.s $D/a458_new.c.a458.s && echo "func_8002A458 cc1 output IDENTICAL" || diff $D/code6cac_b_tu2.c.a458.s $D/a458_new.c.a458.s | head
