#!/bin/bash
# trial_seq.sh <tag> <apply.py>...: scratch clone only, nothing committed. At <tag>, apply each script in turn
# and clean-build after each (exe SHA1 vs the oracle); on a miss, the first differing words (bindiff.py).
H="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile/tmp/q56/adopt"
A="/tmp/q56/adopt tree"; TAG="$1"; shift
cd "$A" && HEAD_REF=$(git rev-parse HEAD) && git checkout -q "$TAG" && source .venv/bin/activate
for ap in "$@"; do
  python3 "$H/$ap" "$A" || { echo "APPLY FAILED $ap"; break; }
  rm -rf build && make -j16 build/bb2.exe >/tmp/q56/trs.log 2>&1
  S=$(sha1sum build/bb2.exe 2>/dev/null | cut -c1-40); echo "$ap: $S"
  if [ "$S" != "62efab4f73f992798c43e8c730aa43baa10bb4fa" ]; then
    grep -E "error|Error|undefined" /tmp/q56/trs.log | head -20
    [ -f build/bb2.exe ] && python3 "$H/bindiff.py" build/bb2.exe disc/SLUS_006.63 2>&1 | head -20
    break
  fi
done
git checkout -q -- . && git clean -qfd -e tmp -e build && git checkout -q q56-adopt && git reset -q --hard "$HEAD_REF"
