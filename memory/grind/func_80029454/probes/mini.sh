#!/bin/bash
# usage: mini.sh snippet.c  -> compiles code6cac_b.c prefix + snippet, prints cc1 asm (post-maspsx) of all snippet functions
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
snip="$1"
out=tmp/func_80029454/mini
mkdir -p $out
line=$(grep -n '^INCLUDE_ASM("asm/funcs", func_80029454)' src/code6cac_b.c | cut -d: -f1)
head -n $((line-1)) src/code6cac_b.c > $out/tu.c
echo '#line 1 "SNIP"' >> $out/tu.c
cat "$snip" >> $out/tu.c
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $out/tu.c | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float > $out/tu.s
# print functions defined in the snippet
for f in $(grep -oE '^[a-zA-Z_][a-zA-Z0-9_ \*]*[ \*]([a-zA-Z_][a-zA-Z0-9_]*)\(' "$snip" | sed -E 's/.*[ \*]([a-zA-Z_][a-zA-Z0-9_]*)\($/\1/'); do
  awk -v f="$f" '$0 ~ "^"f":" {p=1} p {print} p && /\.end/ {exit}' $out/tu.s | grep -v "^\s*\.\(loc\|stabn\|stabs\|frame\|mask\|fmask\)"
done
