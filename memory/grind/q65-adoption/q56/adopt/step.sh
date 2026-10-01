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
