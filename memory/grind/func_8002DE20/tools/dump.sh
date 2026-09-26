#!/bin/bash
# usage: bash tmp/func_8002DE20/dump.sh <tag> <body.c>
# TU = HEAD's src/code6cac_b.c with the INCLUDE_ASM line for func_8002DE20
# replaced by <body.c> (exactly the landing splice). Then:
#  - tools/rtl_track/dump.py (cc1 -da with engine.buildconfig CC_FLAGS) -> .flow/.lreg/.greg
#  - instrumented tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1 -> .alloc (global.c order)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; BODY=$2
D=tmp/func_8002DE20/rtl
mkdir -p $D
python3 - "$BODY" "$D/$TAG.c" <<'EOF'
import subprocess, sys
src = subprocess.run(["git", "show", "HEAD:src/code6cac_b.c"], capture_output=True, text=True, check=True).stdout
line = 'INCLUDE_ASM("asm/funcs", func_8002DE20);\n'
assert src.count(line) == 1
body = open(sys.argv[1]).read()
open(sys.argv[2], "w", newline="\n").write(src.replace(line, body if body.endswith("\n") else body + "\n"))
EOF
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin $(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)') $D/$TAG.c > $D/$TAG.i 2>/dev/null
python3 tools/rtl_track/dump.py de20_$TAG --input $D/$TAG.i > /dev/null
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
d = "tmp/rtl/de20_%s/" % tag
for ext in ("greg", "lreg", "flow"):
    txt = open(d + "in.i." + ext).read()
    m = re.search(r"\n;; Function func_8002DE20\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    open("tmp/func_8002DE20/rtl/%s.%s" % (tag, ext), "w").write(m.group(1) if m else "NOTFOUND")
EOF
cd $D
FLAGS=$(cd ../../.. && python3 -c 'from engine import buildconfig as c; print(c.CC_FLAGS)')
echo "CC_FLAGS=$FLAGS" > $TAG.cmd
BB2_ALLOC_DEBUG=1 ../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o $TAG.inst.s 2> $TAG.alloc.log || true
grep "func=func_8002DE20" $TAG.alloc.log | grep -v seed_used > $TAG.alloc || true
echo "dumped $TAG: $(wc -l < $TAG.lreg) lreg lines, $(wc -l < $TAG.alloc) alloc lines"
