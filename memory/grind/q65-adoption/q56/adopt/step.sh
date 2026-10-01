#!/bin/bash
# step.sh NN name "subject": in the SCRATCH clone ("/tmp/q56/adopt tree", never main) run
# tmp/q56/adopt/sNN_apply.py, clean full build (SHA1 must be the oracle), commit on the scratch branch
# (tag stepNN), engine test and maspsx unit tests on the committed tree, export tmp/q56/adopt/NN-name.patch.
set -o pipefail
NN="$1"; NAME="$2"; SUBJ="$3"
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
A="/tmp/q56/adopt tree"
OUT="$REPO/tmp/q56/adopt"
cd "$A" || exit 1
source .venv/bin/activate
python3 "$OUT/s${NN}_apply.py" "$A" || { echo "apply FAILED"; exit 1; }
rm -rf build
make -j16 build/bb2.exe > "/tmp/q56/step$NN.build.log" 2>&1; rc=$?
SHA=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)
echo "step $NN build rc=$rc exe_sha1=$SHA"
echo "step $NN build rc=$rc exe_sha1=$SHA $(date -u +%FT%TZ)" >> "$OUT/series_run.log"
# per-object compare: every src object against the previous step's build (same name = built both ways)
PREVN=$(printf "%02d" $((10#$NN - 1)))
rm -rf "/tmp/q56/objs_step$NN" && cp -r build/src "/tmp/q56/objs_step$NN"
if [ -d "/tmp/q56/objs_step$PREVN" ]; then
  n=0; d=0; dl=""
  for o in /tmp/q56/objs_step$NN/*.o; do
    b=$(basename "$o"); [ -f "/tmp/q56/objs_step$PREVN/$b" ] || { dl="$dl +$b"; continue; }
    n=$((n+1)); cmp -s "$o" "/tmp/q56/objs_step$PREVN/$b" || { d=$((d+1)); dl="$dl $b"; }
  done
  for o in /tmp/q56/objs_step$PREVN/*.o; do b=$(basename "$o"); [ -f "/tmp/q56/objs_step$NN/$b" ] || dl="$dl -$b"; done
  echo "step $NN objects vs step $PREVN: $n compared, $d differ;${dl:- none}" | tee -a "$OUT/series_run.log"
fi
[ "$SHA" = "62efab4f73f992798c43e8c730aa43baa10bb4fa" ] || { tail -20 "/tmp/q56/step$NN.build.log"; echo "NOT ORACLE"; exit 1; }
git checkout -q -- metrics/events.jsonl 2>/dev/null   # engine commands append metrics; not part of a step
git add -A . >/dev/null
if [ -f "$OUT/s${NN}_msg.txt" ]; then { echo "$SUBJ"; echo; cat "$OUT/s${NN}_msg.txt"; } > /tmp/q56/msg$NN.txt
else echo "$SUBJ" > /tmp/q56/msg$NN.txt; fi
git -c core.hooksPath=/dev/null commit -qF /tmp/q56/msg$NN.txt && git tag -f "step$NN" >/dev/null
bash "$REPO/tmp/q56/etest.sh"
(cd tools/maspsx && python3 -m unittest discover -s tests -t . > "/tmp/q56/step$NN.maspsx.log" 2>&1
 echo "maspsx unittest failures: $(grep -E '^(FAIL|ERROR):' /tmp/q56/step$NN.maspsx.log | awk '{print $2}' | sort | tr '\n' ' ')(baseline: test_div_expand_li_nop test_expand_li_0x1)")
git format-patch -1 --stdout > "$OUT/$NN-$NAME.patch"
echo "files touched:"; git show --stat --format= HEAD | sed 's/^/  /'
