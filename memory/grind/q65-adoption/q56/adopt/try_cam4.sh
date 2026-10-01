#!/bin/bash
# try_cam4.sh: test clone - camera_CalcAngles writes [1] before [0] (value of [0] computed first); rebuild.
cd /tmp/q56r2/t || exit 1
python3 - <<'PY'
p = "src/text1b.c"
t = open(p).read()
a = "    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);\n    D_800A33C8[1] = s0;\n"
assert t.count(a) == 1
t = t.replace(a, "    {\n        s16 neg = -ratan2(sp18[1], sp18[2]);\n        D_800A33C8[1] = s0;\n        D_800A33C8[0] = neg;\n    }\n")
open(p, "w").write(t)
PY
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/tbuild.sh" | head -1
git checkout -q -- src/text1b.c
