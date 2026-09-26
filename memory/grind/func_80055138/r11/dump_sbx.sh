#!/bin/bash
# usage: bash tmp/func_80055138/r11/dump_sbx.sh <tag>
# Dumps the LAST sandboxed TU (tmp/sandbox/func_80055138/src/text1b.c, the landed tree with the
# --candidate body spliced): cpp -> tools/rtl_track/dump.py (build cc1 -da) -> func_80055138's
# .cse/.cse2/.flow/.combine/.lreg/.greg, plus the build cc1's asm for the function.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
TAG=$1
R=tmp/func_80055138/r11/rtl
mkdir -p $R
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin $(python3 -c 'from engine import buildconfig as c; print(c.CPP_DEFS)') tmp/sandbox/func_80055138/src/text1b.c > $R/$TAG.i 2>/dev/null
python3 tools/rtl_track/dump.py r55138_$TAG --input $R/$TAG.i > /dev/null
python3 - "$TAG" <<'EOF'
import sys, re
tag = sys.argv[1]
d = "tmp/rtl/r55138_%s/" % tag
for ext in ("greg", "lreg", "flow", "combine"):
    txt = open(d + "in.i." + ext).read()
    m = re.search(r"\n;; Function func_80055138\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    open("tmp/func_80055138/r11/rtl/%s.%s" % (tag, ext), "w").write(m.group(1) if m else "NOTFOUND")
s = open(d + "in.s").read()
m = re.search(r"\nfunc_80055138:\n(.*?)\.end\s+func_80055138", s, re.S)
open("tmp/func_80055138/r11/rtl/%s.fn.s" % tag, "w").write(m.group(1))
EOF
