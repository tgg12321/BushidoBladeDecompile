#!/bin/bash
# dump.sh <name> [cc1 flags]: tmp/func_80055B60/r11/<name>.c spliced into text1b (mksrc.py: + func_80055138's
# unk_3D0 respelling), cpp with the landing header (tmp/func_80055B60/inc/include ahead of include/), build cc1 -da;
# func_80055B60's dumps in tmp/func_80055B60/r11/d_<name>/
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
n="$1"; shift
out=tmp/func_80055B60/r11/d_$n; rm -rf "$out"; mkdir -p "$out"
python3 tmp/func_80055B60/hdr.py tmp/func_80055B60/inc/include/code6cac.h tmp/func_80055B60/d6a/include/code6cac.h >/dev/null
python3 tmp/func_80055B60/mksrc.py tmp/func_80055B60/r11/$n.c "$out/tu.c" >/dev/null
mipsel-linux-gnu-cpp -Itmp/func_80055B60/inc/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$out/tu.c" > "$out/tu.i"
cd "$out"
../../../../tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$@" -da tu.i -o tu.s 2>cc1.err || { cat cc1.err; exit 1; }
awk '/\.ent\tfunc_80055B60/,/\.end\tfunc_80055B60/' tu.s > func.s
for f in tu.i.*; do awk '/^;; Function /{p=($3=="func_80055B60")} p' $f > f.${f#tu.i.}; rm $f; done
grep -m1 "\.frame" func.s
