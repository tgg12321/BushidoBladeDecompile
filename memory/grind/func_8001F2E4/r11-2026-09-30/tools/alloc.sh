#!/bin/bash
# usage: alloc.sh <candidate.c> <outdir>
# Adapted from memory/grind/func_800198D0/r11/tools/alloc.sh: substitute the candidate body for
# func_8001F2E4 into a copy of src/code6cac_tu2.c, preprocess with the build's cpp flags, run the
# INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1) with the build flags + -dr -dl -dg and BB2_ALLOC_DEBUG=1,
# and also the BUILD cc1 (tools/gcc-2.7.2/build/cc1) on the same .i to check the .s is identical.
set -e
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
CAND="$1"; OUT="$2"
mkdir -p "$OUT"
python3 - "$CAND" "$OUT/tu.c" <<'PY'
import sys
sys.path.insert(0, '.')
from engine import inlineasm
base = open('src/code6cac_tu2.c').read()
body = open(sys.argv[1]).read()
open(sys.argv[2], 'w').write(inlineasm.substitute_body(base, 'func_8001F2E4', body))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx "$OUT/tu.c" > "$OUT/tu.i"
cd "$OUT"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
env BB2_ALLOC_DEBUG=1 "$ROOT/tools/gcc-2.7.2/cc1" $FLAGS -dr -dl -dg tu.i -o tu.s 2> alloc_all.txt || true
"$ROOT/tools/gcc-2.7.2/build/cc1" $FLAGS tu.i -o tu.build.s 2>/dev/null || true
if diff <(grep -v '^ # ' tu.s | grep -v '^#') <(grep -v '^ # ' tu.build.s | grep -v '^#') > /dev/null; then echo "instcheck: identical" > instcheck.txt; else echo "instcheck: DIFFERENT" > instcheck.txt; fi
grep "func=func_8001F2E4" alloc_all.txt > alloc.txt || true
for f in tu.i.rtl tu.i.lreg tu.i.greg; do
  python3 - "$f" <<'PY'
import sys, re
p = sys.argv[1]
s = open(p).read()
m = re.search(r'\n;; Function func_8001F2E4\n', s)
if not m:
    sys.exit(0)
e = s.find('\n;; Function ', m.end())
open(p + '.fn', 'w').write(s[m.start():e if e > 0 else len(s)])
PY
done
cat instcheck.txt; wc -l < alloc.txt
