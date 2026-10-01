#!/bin/bash
# series.sh <main-commit> [first-step]: build the whole Q65 adoption series in a fresh SCRATCH clone of main
# ("/tmp/q56/adopt tree", git clone --shared; never a worktree, never main itself), one commit per step,
# each: full clean build == oracle SHA1, engine test, maspsx unit tests; patches to tmp/q56/adopt/NN-*.patch.
set -o pipefail
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56"
FIRST="${2:-01}"
if [ "$FIRST" = "01" ]; then
  bash "$H/adopt_setup.sh" "$1" || exit 1
  printf '/.venv\n/disc\n/tools/gcc-2.7.2\n/tmp\n' >> "/tmp/q56/adopt tree/.git/info/exclude"
fi
steps=(
 "01|maspsx-indexed-operand-gp|substrate: maspsx _uses_gp - an indexed sym(\$reg) operand is never gp (owner ruling Q65, step 1)"
 "02|b_tu3-boundary-move|src: code6cac_b_tu2 / code6cac_b_tu3 boundary moves to func_800343F0 (owner ruling Q65, step 2, cut outcome (i))"
 "03|text1b_tu1c-boundary-move|src: text1b / text1b_tu1c boundary moves to func_80060A68 (owner ruling Q65, step 3, boundary move in the recorded window)"
 "04|text1a_c-split|src: text1a_c split before func_80044800 into text1a_c_tu2 (owner ruling Q65, step 4)"
 "05|merge-code6cac_b2|src: code6cac_b2_pre + replay_camera_rob_back_loose2 + code6cac_b2_post are one file (owner ruling Q65, step 5)"
 "06|reconcile-c2-config-decls|src: code6cac_c2 / config declarations of func_8004153C and D_800A3708 reconciled (owner ruling Q65, step 6)"
 "07|merge-code6cac_c2-config|src: code6cac_c2 + config are one file (owner ruling Q65, step 7)"
 "08|reconcile-text1b-decls|src: text1a_c2 / text1a_b / text1a_b_pre_rodata / sound / text1b declarations reconciled by evidence (owner rulings Q65/Q67, step 8)"
 "09|merge-text1b|src: text1a_c2 + text1a_b + text1a_b_pre_rodata + sound + text1b are one file (owner rulings Q65/Q67, step 9)"
 "10|reconcile-text1b_b-decls|src: text1b_tu2 / text1b_b (and the text1b_tu1d definition) declarations reconciled by evidence (owner rulings Q65/Q67, step 10)"
 "11|merge-text1b_b|src: text1b_tu2 + text1b_b are one file (owner rulings Q65/Q67, step 11)"
 "12|maspsx-static-lcomm|substrate: maspsx models .local+.comm (an uninitialized static) as .lcomm (owner ruling Q65, step 12)"
 "13|maspsx-small-data-sdata|substrate: maspsx models cc1psx -G8's .sdata choice for small initialized objects (owner ruling Q68, step 13)"
 "14|reconcile-static-decls|src: one declaration each for D_800A3468 (text1b_tu1c) and g_anim_hit_flags (text1a_post) (owner ruling Q65, step 14)"
 "15|per-file-gp-switch|src/build: per-file gp model - definitions follow the evidence, data blob cut, maspsx -G8 per file, sdata lists retired (owner rulings Q65/Q67, step 15)"
 "16|tooling-records|tools/records: follow the retired sdata lists and the moved functions (owner ruling Q65, step 16)"
)
for s in "${steps[@]}"; do
  IFS='|' read -r NN NAME SUBJ <<< "$s"
  [[ "$NN" < "$FIRST" ]] && continue
  echo "===== step $NN $NAME"
  bash "$H/adopt/step.sh" "$NN" "$NAME" "$SUBJ" 2>&1 | grep -v "missing .end" | tail -30 || exit 1
  [ -f "$H/adopt/$NN-$NAME.patch" ] || { echo "step $NN produced no patch"; exit 1; }
  cd "/tmp/q56/adopt tree" && git describe --tags --exact-match HEAD 2>/dev/null | grep -q "step$NN" || { echo "step $NN not committed"; exit 1; }
done
echo "series complete"
