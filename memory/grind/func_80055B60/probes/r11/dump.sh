#!/bin/bash
# dump.sh <name> : splice tmp/b60/<name>.c over func_80055B60's INCLUDE_ASM in a copy of src/text1b.c,
# compile with the build cc1 (-da) using the fix1 game.h; dumps in tmp/b60/d_<name>/
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
n="$1"; shift
out=tmp/b60/d_$n; rm -rf "$out"; mkdir -p "$out"
python3 - tmp/b60/$n.c "$out/tu.c" <<'EOF'
import sys
src = open('src/text1b.c').read()
line = 'INCLUDE_ASM("asm/funcs", func_80055B60);'
assert src.count(line) == 1
open(sys.argv[2], 'w').write(src.replace(line, open(sys.argv[1]).read()))
EOF
mipsel-linux-gnu-cpp -Itmp/b60/inc/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$out/tu.c" > "$out/tu.i"
cd "$out"
../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$@" -da tu.i -o tu.s 2>cc1.err || { cat cc1.err; exit 1; }
awk '/\.ent\tfunc_80055B60/,/\.end\tfunc_80055B60/' tu.s > func.s
for f in tu.i.*; do awk '/^;; Function /{p=($3=="func_80055B60")} p' $f > f.${f#tu.i.}; rm $f; done
grep -m1 "\.frame" func.s
