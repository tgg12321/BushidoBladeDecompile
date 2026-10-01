#!/bin/bash
# try_cam5.sh <variant>: test clone (post-switch, D_800A33C8[2] static): respell camera_CalcAngles' tail; build.
cd /tmp/q56r2/t || exit 1
cp src/text1b.c /tmp/q56r2/text1b.keep.c
python3 - "$1" <<'PY'
import sys
p = "src/text1b.c"
t = open(p).read()
a = "    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);\n    D_800A33C8[1] = s0;\n    return D_800A33C8;\n"
assert t.count(a) == 1
V = {
 "dowhile": "    do {\n        D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);\n        D_800A33C8[1] = s0;\n    } while (0);\n    return D_800A33C8;\n",
 "amp0": "    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);\n    D_800A33C8[1] = s0;\n    return &D_800A33C8[0];\n",
 "neg": "    {\n        s16 neg = -ratan2(sp18[1], sp18[2]);\n        D_800A33C8[0] = neg;\n    }\n    D_800A33C8[1] = s0;\n    return D_800A33C8;\n",
}
open(p, "w").write(t.replace(a, V[sys.argv[1]]))
PY
echo "$1: $(bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/tbuild.sh" | head -1)"
cp /tmp/q56r2/text1b.keep.c src/text1b.c
