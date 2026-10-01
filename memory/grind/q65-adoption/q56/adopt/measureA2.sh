#!/bin/bash
# measureA2.sh: option (A) measurement, post-switch. Test clone at the chain's t13 commit; s14 with every A8 join
# except D_800A33C8/D_800A33CA; s15 (pre-switch oracle holds); then the D_800A33C8 join done after the switch
# (static s16 D_800A33C8[2] in text1b, camera_CalcAngles respelled) and a full build with and without text1b
# in GP_FILES (cc1 -G8).
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd /tmp/q56r2/t || exit 1
source .venv/bin/activate
T13=$(git log --format=%H --grep='^t13$' | head -1)
[ -n "$T13" ] || T13=$(git reflog --format='%H %gs' | awk '$0 ~ /commit: t13$/{print $1; exit}')
echo "t13=$T13"
git checkout -q -- . && git clean -qfd -e tmp -e build -e .venv -e disc -e tools/gcc-2.7.2
git reset -q --hard "$T13" && git log -1 --format=%s
sed 's/^A8_PAIRS = \[("D_800A33C8", "D_800A33CA"), /A8_PAIRS = [/' "$REPO/tmp/q56/adopt/s14_apply.py" > "$REPO/tmp/q56/adopt/s14_meas.py"
python3 "$REPO/tmp/q56/adopt/s14_meas.py" /tmp/q56r2/t > /tmp/q56r2/mA14.log 2>&1 || { echo s14 failed; tail -3 /tmp/q56r2/mA14.log; exit 1; }
rm -f "$REPO/tmp/q56/adopt/s14_meas.py"
echo "s14 (4 joins): $(bash "$REPO/tmp/q56/r2/tbuild.sh" | head -1)"
python3 "$REPO/tmp/q56/adopt/s15_apply.py" /tmp/q56r2/t > /tmp/q56r2/mA15.log 2>&1; echo "s15 rc=$?"; tail -1 /tmp/q56r2/mA15.log
echo "s15, D_800A33C8/CA as two statics: $(bash "$REPO/tmp/q56/r2/tbuild.sh" | head -1)"
grep -n "D_800A33C[8A]" src/text1b.c | head
python3 - <<'PY'
import re
p = "src/text1b.c"
t = open(p).read()
t = re.sub(r"^static s16 D_800A33C8;\n", "static s16 D_800A33C8[2];\n", t, flags=re.M)
t = re.sub(r"^static s16 D_800A33CA;[^\n]*\n", "", t, flags=re.M)
t = t.replace("    D_800A33C8 = -ratan2(sp18[1], sp18[2]);\n    D_800A33CA = s0;\n    return &D_800A33C8;\n",
              "    D_800A33C8[0] = -ratan2(sp18[1], sp18[2]);\n    D_800A33C8[1] = s0;\n    return D_800A33C8;\n")
open(p, "w").write(t)
PY
grep -n "D_800A33C[8A]" src/text1b.c | head
echo "joined after the switch, cc1 -G0: $(bash "$REPO/tmp/q56/r2/tbuild.sh" | head -2)"
sed -i 's/^GP_FILES := \(.*\)$/GP_FILES := \1 text1b/' Makefile
echo "joined after the switch, text1b cc1 -G8: $(bash "$REPO/tmp/q56/r2/tbuild.sh" | head -2)"
