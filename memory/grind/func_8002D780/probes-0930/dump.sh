#!/bin/bash
# RTL dumps (instrumented cc1) of a func_8002D780 candidate spliced into a COPY of src/code6cac_b_tu2.c.
# usage: dump.sh <candidate.c> <outdir>
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
CAND="$1"; OUT="$2"
mkdir -p "$OUT"
python3 - "$CAND" "$OUT/tu.c" <<'EOF'
import sys
src = open("src/code6cac_b_tu2.c", encoding="utf-8").read()
cand = open(sys.argv[1], encoding="utf-8").read()
key = 'INCLUDE_ASM("asm/funcs", func_8002D780);'
assert src.count(key) == 1
open(sys.argv[2], "w", encoding="utf-8", newline="\n").write(src.replace(key, cand.rstrip("\n")))
EOF
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$OUT/tu.c" > "$OUT/tu.i"
cd "$OUT"
../../../tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
  -df -dc -dl -dg tu.i -o tu.s
ls
