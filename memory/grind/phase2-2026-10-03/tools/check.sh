#!/bin/bash
# check.sh [--clean] BASE NEW : the Phase 2 byte-identity gate (run in WSL; any cwd).
#   1. make (every header is a dependency of every C object, so a plain make rebuilds what an edit
#      touches; --clean forces make clean first)
#   2. the linked EXE's SHA1 against the oracle
#   3. snap.py NEW: every linked object, per-TU token hashes, implicit-declaration pairs, sources
#   4. cmp.py BASE NEW: per-object section bytes / relocations / symbols; implicit-set changes
#   5. l2diff.py BASE NEW: layer-2 body keys that moved between the two snapshots' sources
# `check.sh --base NAME` only builds and snapshots (take the base on an unchanged tree).
# Scratch: tmp/p2/. Exit 0 only if the SHA1 matches and every object is identical.
# (Moved layer-2 keys are reported, not failed: a typed rewrite moves keys by design; each moved
#  key names a body that needs its layer-2 review re-run before the commit.)
set -u -o pipefail
cd "$(dirname "$0")/../../../.." || exit 2
source .venv/bin/activate
T=memory/grind/phase2-2026-10-03/tools
ORACLE=62efab4f73f992798c43e8c730aa43baa10bb4fa
mkdir -p tmp/p2
clean=0; baseonly=0
while [ $# -gt 0 ]; do
  case "$1" in
    --clean) clean=1; shift ;;
    --base) baseonly=1; shift ;;
    *) break ;;
  esac
done
if [ $baseonly = 1 ]; then
  [ $# = 1 ] || { echo "usage: check.sh [--clean] --base NAME | check.sh [--clean] BASE NEW"; exit 2; }
  NEW=$1; BASE=
else
  [ $# = 2 ] || { echo "usage: check.sh [--clean] --base NAME | check.sh [--clean] BASE NEW"; exit 2; }
  BASE=$1; NEW=$2
  [ -d "tmp/p2/snap/$BASE" ] || { echo "no base snapshot tmp/p2/snap/$BASE"; exit 2; }
fi
log=tmp/p2/$NEW.make.log
[ $clean = 1 ] && make clean > /dev/null
make -j"$(nproc)" > "$log" 2>&1
mk=$?
sha=$(sha1sum build/bb2.exe 2>/dev/null | cut -d' ' -f1)
if [ "$sha" = "$ORACLE" ]; then echo "SHA1 OK $sha"; else echo "SHA1 MISMATCH ${sha:-<no exe>} (make exit $mk; log $log)"; tail -15 "$log"; fi
[ -f build/bb2.exe ] || exit 2
python3 $T/snap.py "$NEW" || exit 2
[ $baseonly = 1 ] && { [ "$sha" = "$ORACLE" ]; exit $?; }
python3 $T/cmp.py "$BASE" "$NEW"; c=$?
python3 $T/l2diff.py "$BASE" "$NEW"
[ "$sha" = "$ORACLE" ] && [ $c = 0 ] && { echo "CHECK PASS"; exit 0; }
echo "CHECK FAIL"; exit 1
