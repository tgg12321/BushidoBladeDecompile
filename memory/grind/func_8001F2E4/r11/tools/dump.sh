#!/bin/bash
# dump.sh <body.c> <outdir> [findreg-pseudo ...]
# Substitute <body.c> for func_8001F2E4 into a COPY of src/code6cac_tu2.c (src is only read),
# preprocess with the build's cpp flags (engine/buildconfig.py CPP_FLAGS + CPP_DEFS), run the
# INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1) with the build's CC_FLAGS (code6cac_tu2 is a -G0 file)
# plus -dr -ds -dt -df -dc -dl -dg, BB2_ALLOC_DEBUG=1 (global.c allocation order/priority), and
# BB2_FINDREG_DEBUG=<pseudo> for each pseudo named (global.c find_reg exclusion sets). The BUILD
# cc1 (tools/gcc-2.7.2/build/cc1) is run on the same .i and its .s compared with the instrumented
# one (instcheck.txt). Every dump is cut to func_8001F2E4 (<dump>.fn).
set -e
ROOT="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$ROOT"
CAND="$1"; OUT="$2"; shift 2
mkdir -p "$OUT"
python3 - "$CAND" "$OUT/tu.c" <<'PY'
import sys
sys.path.insert(0, '.')
from engine import inlineasm
base = open('src/code6cac_tu2.c').read()
body = open(sys.argv[1]).read()
open(sys.argv[2], 'w').write(inlineasm.substitute_body(base, 'func_8001F2E4', body))
PY
CPPDEFS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CPP_FLAGS, getattr(b,'CPP_DEFS',''))")
FLAGS=$(python3 -c "import sys; sys.path.insert(0,'.'); from engine import buildconfig as b; print(b.CC_FLAGS)")
echo "cpp: mipsel-linux-gnu-cpp $CPPDEFS tu.c > tu.i" > "$OUT/cmd.txt"
mipsel-linux-gnu-cpp $CPPDEFS "$OUT/tu.c" > "$OUT/tu.i"
cd "$OUT"
echo "cc1: env BB2_ALLOC_DEBUG=1 [BB2_FINDREG_DEBUG=<p>] tools/gcc-2.7.2/cc1 $FLAGS -dr -ds -dt -df -dc -dl -dg tu.i -o tu.s" >> cmd.txt
env BB2_ALLOC_DEBUG=1 "$ROOT/tools/gcc-2.7.2/cc1" $FLAGS -dr -ds -dt -df -dc -dl -dg tu.i -o tu.s 2> alloc_all.txt
grep "func=func_8001F2E4" alloc_all.txt > alloc.txt || true
for p in "$@"; do
  env BB2_FINDREG_DEBUG=$p "$ROOT/tools/gcc-2.7.2/cc1" $FLAGS tu.i -o /dev/null 2> fr_all.txt
  grep -A12 "FINDREGDBG func=func_8001F2E4 pseudo=$p " fr_all.txt > findreg_$p.txt || true
  echo "findreg: env BB2_FINDREG_DEBUG=$p tools/gcc-2.7.2/cc1 $FLAGS tu.i" >> cmd.txt
done
rm -f fr_all.txt
"$ROOT/tools/gcc-2.7.2/build/cc1" $FLAGS tu.i -o tu.build.s
if diff <(grep -v '^ #' tu.s | grep -v '^#') <(grep -v '^ #' tu.build.s | grep -v '^#') > /dev/null; then echo "instcheck: identical" > instcheck.txt; else echo "instcheck: DIFFERENT" > instcheck.txt; fi
for f in tu.i.rtl tu.i.cse tu.i.cse2 tu.i.flow tu.i.combine tu.i.lreg tu.i.greg; do
  [ -f "$f" ] || continue
  python3 - "$f" <<'PY'
import sys, re
p = sys.argv[1]
s = open(p).read()
m = re.search(r'\n;; Function func_8001F2E4\n', s)
if m:
    e = s.find('\n;; Function ', m.end())
    open(p + '.fn', 'w').write(s[m.start():e if e > 0 else len(s)])
PY
  rm -f "$f"
done
cat instcheck.txt
