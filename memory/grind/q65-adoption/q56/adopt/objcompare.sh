#!/bin/bash
# objcompare.sh TAG_A TAG_B...: scratch clone only. For each consecutive pair of tags, build both trees and
# compare EVERY src/*.c object byte for byte (the rule's check for a maspsx or flag change), then return the
# clone to its branch head.
A="/tmp/q56/adopt tree"; cd "$A" && source .venv/bin/activate
HEAD_REF=$(git rev-parse HEAD)
prev=""
for t in "$@"; do
  git checkout -q "$t" && rm -rf build && make -j16 build/bb2.exe > /tmp/q56/objcmp.log 2>&1
  rm -rf "/tmp/q56/objs_$t" && cp -r build/src "/tmp/q56/objs_$t"
  if [ -n "$prev" ]; then
    n=0; d=0
    for o in /tmp/q56/objs_$t/*.o; do
      b=$(basename "$o"); n=$((n+1))
      cmp -s "$o" "/tmp/q56/objs_$prev/$b" || { d=$((d+1)); echo "  differs: $b"; }
    done
    echo "$prev -> $t: $n objects compared, $d differ"
  fi
  prev="$t"
done
git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
