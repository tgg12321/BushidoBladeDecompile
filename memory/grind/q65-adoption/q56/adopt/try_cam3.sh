#!/bin/bash
# try_cam3.sh: test clone - D_800A33C8 as a two-halfword struct in camera_CalcAngles; rebuild, SHA1.
cd /tmp/q56r2/t || exit 1
python3 - <<'PY'
p = "src/text1b.c"
t = open(p).read()
t = t.replace("extern s16 D_800A33C8[2];", "typedef struct { s16 a; s16 b; } Pair_800A33C8;\nextern Pair_800A33C8 D_800A33C8;")
t = t.replace("    *D_800A33C8 = -ratan2(sp18[1], sp18[2]);", "    D_800A33C8.a = -ratan2(sp18[1], sp18[2]);")
t = t.replace("    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);", "    D_800A33C8.a = -ratan2(sp18[1], sp18[2]);")
t = t.replace("    D_800A33C8[1] = s0;", "    D_800A33C8.b = s0;")
t = t.replace("    return D_800A33C8;", "    return &D_800A33C8.a;")
open(p, "w").write(t)
PY
grep -n "D_800A33C8" src/text1b.c
bash "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/r2/tbuild.sh" | head -1
