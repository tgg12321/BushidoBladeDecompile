#!/bin/bash
# usage: bash tmp/func_8003993C/r11/dump.sh <tag> <body.c>
# TU = HEAD's src/code6cac_c_mid.c with the INCLUDE_ASM line for func_8003993C
# replaced by <body.c> (exactly the landing splice); headers = HEAD's include/
# (git archive, so another worker's uncommitted header edit cannot leak in). Then:
#  - tools/rtl_track/dump.py (cc1 -da, engine.buildconfig CC_FLAGS) -> .flow/.lreg/.greg (+ .combine/.loop/.cse2)
#  - instrumented tools/gcc-2.7.2/cc1 with BB2_ALLOC_DEBUG=1 -> .alloc (global.c order)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1; BODY=$2
D=tmp/func_8003993C/r11/rtl
H=tmp/func_8003993C/r11/hdr
mkdir -p $D
if [ ! -d $H/include ]; then mkdir -p $H && git archive HEAD include | tar -x -C $H; fi
python3 - "$BODY" "$D/$TAG.c" <<'EOF'
import subprocess, sys
src = subprocess.run(["git", "show", "HEAD:src/code6cac_c_mid.c"], capture_output=True, text=True, check=True).stdout
line = 'INCLUDE_ASM("asm/funcs", func_8003993C);\n'
assert src.count(line) == 1
body = open(sys.argv[1]).read()
open(sys.argv[2], "w", newline="\n").write(src.replace(line, body if body.endswith("\n") else body + "\n"))
EOF
mipsel-linux-gnu-cpp -I$H/include -undef -Wall -lang-c -fno-builtin $(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)') $D/$TAG.c > $D/$TAG.i 2>/dev/null
python3 tools/rtl_track/dump.py c93c_$TAG --input $D/$TAG.i > /dev/null
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
d = "tmp/rtl/c93c_%s/" % tag
for ext in ("greg", "lreg", "flow", "combine", "loop", "cse2", "rtl"):
    try:
        txt = open(d + "in.i." + ext).read()
    except FileNotFoundError:
        continue
    m = re.search(r"\n;; Function func_8003993C\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    open("tmp/func_8003993C/r11/rtl/%s.%s" % (tag, ext), "w").write(m.group(1) if m else "NOTFOUND")
EOF
cd $D
FLAGS=$(cd ../../../.. && python3 -c 'from engine import buildconfig as c; print(c.CC_FLAGS)')
echo "CC_FLAGS=$FLAGS" > $TAG.cmd
BB2_ALLOC_DEBUG=1 ../../../../tools/gcc-2.7.2/cc1 $FLAGS $TAG.i -o $TAG.inst.s 2> $TAG.alloc.log || true
grep "func=func_8003993C" $TAG.alloc.log | grep -v seed_used > $TAG.alloc || true
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
s = open(tag + ".inst.s").read()
m = re.search(r"\nfunc_8003993C:\n(.*?)\n\t\.end\tfunc_8003993C", s, re.S)
open(tag + ".fn.s", "w").write(m.group(1) if m else "NOTFOUND")
EOF
echo "dumped $TAG: $(wc -l < $TAG.lreg) lreg lines, $(wc -l < $TAG.alloc) alloc lines"
