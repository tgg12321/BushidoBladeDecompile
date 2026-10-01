#!/bin/bash
# try_cam.sh <replacement-for-return-line>: test clone - respell camera_CalcAngles' return, rebuild, SHA1.
cd /tmp/q56r2/t || exit 1
python3 - "$1" <<'PY'
import sys
p = "src/text1b.c"
t = open(p).read()
a = t.index("    D_800A33C8[1] = s0;\n")
b = t.index("\n", t.index("    return", a)) + 1
t = t[:a] + "    D_800A33C8[1] = s0;\n" + sys.argv[1] + "\n" + t[b:]
open(p, "w").write(t)
PY
grep -n "D_800A33C8\[1\] = s0;" -A2 src/text1b.c
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/tbuild.sh" | head -1
