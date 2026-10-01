#!/bin/bash
# dump.sh <name> <cand.c>: in the private clone, splice the candidate into src/text1b_b.c (copy), cc1 -da,
# extract func_800770B8's section of each dump into /tmp/l770/d/<name>/f.<pass>, plus the .s.
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
N="$1"; C="$2"; case "$C" in /*) ;; *) C="$R/$C";; esac
cd /tmp/l770/tree || exit 1
D=/tmp/l770/d/$N; rm -rf "$D"; mkdir -p "$D"
python3 - "$C" "$D/x.c" <<'PY'
import sys, re
src = open("src/text1b_b.c").read()
cand = open(sys.argv[1]).read()
line = 'INCLUDE_ASM("asm/funcs", func_800770B8);'
assert src.count(line) == 1
open(sys.argv[2], "w").write(src.replace(line, cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$D/x.c" > "$D/x.i"
(cd "$D" && /tmp/l770/tree/tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da -dumpbase x x.i -o x.s)
for f in "$D"/x.*; do
  ext="${f##*.}"; case "$ext" in c|i) continue;; esac
  awk '/^;; Function /{p=($3=="func_800770B8")} p' "$f" > "$D/f.$ext"
done
awk '/^func_800770B8:/{p=1} p{print} p&&/\.end[ \t]+func_800770B8/{exit}' "$D/x.s" > "$D/f.asm"
ls "$D" | tr '\n' ' '; echo
