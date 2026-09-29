#!/bin/bash
# usage: alloc.sh <candidate.c> <outdir> [extra env assignments...]
# Substitutes candidate into src/code6cac.c copy, preprocesses, runs the
# INSTRUMENTED cc1 with BB2_ALLOC_DEBUG + -dl -dg (+ -g for var->reg stabs).
set -e
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
CAND="$1"; OUT="$2"; shift 2
mkdir -p "$OUT"
python3 - "$CAND" "$OUT/code6cac.c" <<'EOF'
import sys
sys.path.insert(0, '.')
from engine import inlineasm
base = open('src/code6cac.c').read()
body = open(sys.argv[1]).read()
open(sys.argv[2], 'w').write(inlineasm.substitute_body(base, 'func_800198D0', body))
EOF
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx "$OUT/code6cac.c" > "$OUT/code6cac.i"
cd "$OUT"
env BB2_ALLOC_DEBUG=1 "$@" "$ROOT/tools/gcc-2.7.2/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dr -dl -dg code6cac.i -o code6cac.s 2> alloc_all.txt || true
grep "func=func_800198D0" alloc_all.txt > alloc.txt || true
for f in code6cac.i.rtl code6cac.i.lreg code6cac.i.greg; do
  python3 - "$f" <<'EOF'
import sys, re
p = sys.argv[1]
s = open(p).read()
m = re.search(r'\n;; Function func_800198D0\n', s)
if not m:
    sys.exit(0)
e = s.find('\n;; Function ', m.end())
open(p + '.fn', 'w').write(s[m.start():e if e > 0 else len(s)])
EOF
done
wc -l alloc.txt
