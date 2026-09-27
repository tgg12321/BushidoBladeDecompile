#!/bin/bash
# usage: bash tmp/func_80027AD8/r11/dump.sh <tag> <body.c> [findreg pseudos...]
# Splices <body.c> into a scratch copy of src/code6cac_b.c (replacing the jtbl INCLUDE_RODATA +
# INCLUDE_ASM lines, exactly as the landing does), preprocesses it with engine.buildconfig CPP_DEFS,
# runs the BUILD cc1 (tools/gcc-2.7.2/build/cc1, engine.buildconfig CC_FLAGS) with -df -dl -dg -dJ -dS
# and the instrumented tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1 BB2_SUGG_DEBUG=1 (+ one
# BB2_FINDREG_DEBUG run per requested pseudo), and checks the two cc1s emit the same func_80027AD8.
# Outputs: tmp/func_80027AD8/r11/rtl/<tag>.{i,flow,lreg,greg,sched,jump2,alloc,sugg,findreg.N,fn.s}
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; BODY=$2; shift 2
R=tmp/func_80027AD8/r11/rtl; mkdir -p $R/$TAG.d
python3 - "$BODY" "$R/$TAG.d/x.c" <<'EOF'
import sys
body = open(sys.argv[1]).read()
src = open('src/code6cac_b.c').read()
old = 'INCLUDE_RODATA("asm/rodata", jtbl_80010548);\nINCLUDE_ASM("asm/funcs", func_80027AD8);\n'
assert src.count(old) == 1
open(sys.argv[2], 'w').write(src.replace(old, body if body.endswith('\n') else body + '\n'))
EOF
DEFS=$(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)')
FLAGS=$(python3 -c 'from engine import buildconfig as c; print(c.CC_FLAGS)')
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc $DEFS $R/$TAG.d/x.c 2>/dev/null > $R/$TAG.i
cp $R/$TAG.i $R/$TAG.d/x.i
( cd $R/$TAG.d && ../../../../../tools/gcc-2.7.2/build/cc1 $FLAGS -df -dl -dg -dJ -dS x.i -o x.s 2>/dev/null )
python3 - "$R" "$TAG" <<'EOF'
import sys, re
R, tag = sys.argv[1], sys.argv[2]
d = f"{R}/{tag}.d/x.i."
for ext, name in (("flow", "flow"), ("lreg", "lreg"), ("greg", "greg"), ("jump2", "jump2"), ("sched", "sched")):
    try:
        txt = open(d + ext).read()
    except FileNotFoundError:
        continue
    m = re.search(r"\n;; Function func_80027AD8\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    open(f"{R}/{tag}.{name}", "w").write(m.group(1) if m else "NOTFOUND")
s = open(f"{R}/{tag}.d/x.s").read()
m = re.search(r"\nfunc_80027AD8:\n.*?\.end\tfunc_80027AD8", s, re.S)
open(f"{R}/{tag}.fn.s", "w").write(m.group(0))
EOF
cd $R
BB2_ALLOC_DEBUG=1 BB2_SUGG_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o $TAG.inst.s 2> $TAG.dbg.log
grep "ALLOCDBG func=func_80027AD8" $TAG.dbg.log > $TAG.alloc || true
grep "SUGGDBG-QTY func=func_80027AD8" $TAG.dbg.log > $TAG.sugg || true
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
s = open(f"{tag}.inst.s").read()
m = re.search(r"\nfunc_80027AD8:\n.*?\.end\tfunc_80027AD8", s, re.S)
a = open(f"{tag}.fn.s").read()
print("IDENTITY OK" if m and m.group(0) == a else "IDENTITY DIFFERS", len(a.splitlines()), "lines")
EOF
for P in "$@"; do
  BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=$P ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o /dev/null 2> /tmp/fr7ad8.log
  awk -v P="$P" '$0 ~ ("FINDREGDBG func=func_80027AD8 pseudo=" P " ") {f=1} $0 ~ /FINDREGDBG func=/ && $0 !~ ("func=func_80027AD8 pseudo=" P " ") {f=0} f && /FINDREGDBG/ {print}' /tmp/fr7ad8.log > $TAG.findreg.$P
done
echo "flags: $FLAGS"
