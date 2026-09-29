#!/bin/bash
# Splice a candidate into src/text1b.c (copy), preprocess, compile with the build cc1 flags,
# extract func_8005E54C's cc1 asm. Optional: DUMP=1 -> instrumented cc1 RTL dumps (-dc -dL -dl -dg -ds -df).
# Usage: bash tmp/func_8005E54C/dump.sh <candidate.c> <outdir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
CAND=$1
OUT=${2:-tmp/func_8005E54C/d}
mkdir -p "$OUT"
python3 - "$CAND" "$OUT/t.c" <<'EOF'
import sys
src = open('src/text1b.c').read()
cand = open(sys.argv[1]).read()
key = 'INCLUDE_ASM("asm/funcs", func_8005E54C);'
assert key in src
open(sys.argv[2], 'w').write(src.replace(key, cand))
EOF
if [ -n "$PATCH" ]; then python3 "$PATCH" "$OUT/t.c"; fi
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$OUT/t.c" > "$OUT/t.i" 2>/dev/null
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
if [ -n "$DUMP" ]; then
  (cd "$OUT" && ../../../tools/gcc-2.7.2/cc1 $FLAGS -dc -dL -dl -dg -ds -df -dj -dJ t.i -o t.s)
else
  tools/gcc-2.7.2/build/cc1 $FLAGS "$OUT/t.i" -o "$OUT/t.s"
fi
awk '/^func_8005E54C:/{f=1} f{print} f&&/\.end\tfunc_8005E54C/{exit}' "$OUT/t.s" > "$OUT/f.s"
grep -c '^\s[a-z]' "$OUT/f.s" || true
