#!/bin/bash
# usage: bash tmp/func_80055138/r11/dump.sh <tag> <body.c>
# Builds the header-model TU with <body.c> spliced (memory/grind/func_80055138/integration/wf2.py),
# preprocesses it, dumps .flow/.lreg/.greg (tools/rtl_track/dump.py, cc1 -da, engine.buildconfig
# CC_FLAGS), and runs the instrumented tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1
# BB2_FINDREG_DEBUG=${FR:-0}.  Outputs: tmp/func_80055138/r11/rtl/<tag>.{i,flow,lreg,greg,alloc,findreg,inst.s}
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; BODY=$2
R=tmp/func_80055138/r11/rtl
mkdir -p $R
python3 tmp/func_80055138/r11/model.py score "$BODY"
W=tmp/func_80055138/wf3
mipsel-linux-gnu-cpp -I$W/inc -Iinclude -undef -Wall -lang-c -fno-builtin $(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)') $W/text1b.c > $R/$TAG.i 2>/dev/null
python3 tools/rtl_track/dump.py r55138_$TAG --input $R/$TAG.i > /dev/null
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
d = "tmp/rtl/r55138_%s/" % tag
for ext in ("greg", "lreg", "flow"):
    txt = open(d + "in.i." + ext).read()
    m = re.search(r"\n;; Function func_80055138\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    open("tmp/func_80055138/r11/rtl/%s.%s" % (tag, ext), "w").write(m.group(1) if m else "NOTFOUND")
print("dumps ok")
EOF
cd $R
FLAGS=$(cd ../../../.. && python3 -c 'from engine import buildconfig as c; print(c.CC_FLAGS)')
BB2_ALLOC_DEBUG=1 BB2_FINDREG_DEBUG=${FR:-0} ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o $TAG.inst.s 2> $TAG.alloc.log
grep "func=func_80055138" $TAG.alloc.log | grep -v seed_used > $TAG.alloc || true
awk '/FINDREGDBG func=func_80055138/{f=1} /FINDREGDBG func=/&&!/func_80055138/{f=0} f&&/FINDREGDBG/{print}' $TAG.alloc.log > $TAG.findreg
echo "flags: $FLAGS"
wc -l $TAG.alloc $TAG.findreg
# identity: the instrumented cc1's func_80055138 asm == the build cc1's (dump.py in.s)
awk '/^func_80055138:/{f=1} f{print} /\.end[ \t]+func_80055138/{f=0}' ../../../rtl/r55138_$TAG/in.s > $TAG.frozen.fn.s
awk '/^func_80055138:/{f=1} f{print} /\.end[ \t]+func_80055138/{f=0}' $TAG.inst.s > $TAG.inst.fn.s
if cmp -s $TAG.frozen.fn.s $TAG.inst.fn.s; then echo "IDENTITY OK ($(wc -l < $TAG.inst.fn.s) lines)"; else echo "IDENTITY DIFFERS"; fi
