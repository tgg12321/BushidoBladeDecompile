#!/bin/bash
# amend.sh NN name script.py: scratch clone only - apply an addendum script, clean build (oracle required),
# amend the step commit, re-run tests, re-export the patch.
NN="$1"; NAME="$2"; SCRIPT="$3"
REPO="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; A="/tmp/q56/adopt tree"; OUT="$REPO/tmp/q56/adopt"
cd "$A" && source .venv/bin/activate
python3 "$OUT/$SCRIPT" "$A" || exit 1
rm -rf build; make -j16 build/bb2.exe > "/tmp/q56/step$NN.build.log" 2>&1
SHA=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1); echo "exe_sha1=$SHA"
[ "$SHA" = "62efab4f73f992798c43e8c730aa43baa10bb4fa" ] || { tail -20 "/tmp/q56/step$NN.build.log"; echo NOT ORACLE; exit 1; }
git add -A . >/dev/null && git -c core.hooksPath=/dev/null commit -q --amend --no-edit && git tag -f "step$NN" >/dev/null
bash "$REPO/tmp/q56/etest.sh"
(cd tools/maspsx && python3 -m unittest discover -s tests -t . > "/tmp/q56/step$NN.maspsx.log" 2>&1
 echo "maspsx unittest failures: $(grep -E '^(FAIL|ERROR):' /tmp/q56/step$NN.maspsx.log | awk '{print $2}' | sort | tr '\n' ' ')(baseline: test_div_expand_li_nop test_expand_li_0x1)")
git format-patch -1 --stdout > "$OUT/$NN-$NAME.patch"
git show --stat --format= HEAD | sed 's/^/  /'
