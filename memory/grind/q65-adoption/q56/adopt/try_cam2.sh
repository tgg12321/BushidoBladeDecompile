#!/bin/bash
# try_cam2.sh <store-line> <return-line>: test clone - respell camera_CalcAngles' first store and return, rebuild.
cd /tmp/q56r2/t || exit 1
python3 - "$1" "$2" <<'PY'
import sys, re
p = "src/text1b.c"
t = open(p).read()
k = t.index("s16 *camera_CalcAngles(void) {")
e = t.index("\n}\n", k)
body = t[k:e]
L = body.split("\n")
for i, l in enumerate(L):
    if "= -ratan2(sp18[1], sp18[2]);" in l:
        L[i] = sys.argv[1]
    if l.strip().startswith("return"):
        L[i] = sys.argv[2]
t = t[:k] + "\n".join(L) + t[e:]
open(p, "w").write(t)
PY
sed -n "$(grep -n 's16 \*camera_CalcAngles' src/text1b.c | cut -d: -f1),+18p" src/text1b.c | tail -5
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/tbuild.sh" | head -1
