#!/bin/bash
# usage: dump.sh cand.c outdir [cc1-dump-flags...]   -> preprocessed TU + RTL dumps in outdir
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cand="$1"; out="$2"; shift 2
mkdir -p "$out"
line=$(grep -n '^INCLUDE_ASM("asm/funcs", func_80029454)' tmp/func_80029454/head_src.c | cut -d: -f1)
head -n $((line-1)) tmp/func_80029454/head_src.c > "$out/tu.c"
cat "$cand" >> "$out/tu.c"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$out/tu.c" > "$out/tu.i"
CC1=${CC1:-tools/gcc-2.7.2/cc1}
$CC1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$@" "$out/tu.i" -o "$out/tu.s" 2> "$out/cc1.err"
echo "rc=$?"
ls "$out"
