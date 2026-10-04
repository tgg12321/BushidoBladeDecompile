#!/bin/bash
# Determinism check: N clean builds, snapshot every object, compare.
set -u
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
OUT=tmp/s6/det
rm -rf "$OUT"; mkdir -p "$OUT"
run() {
  local name=$1; shift
  local t0=$(date +%s)
  make clean >/dev/null
  "$@" > "$OUT/$name.log" 2>&1; local rc=$?
  echo "$name rc=$rc secs=$(( $(date +%s) - t0 ))" | tee -a "$OUT/summary.txt"
  (cd build && find . -name '*.o' -o -name 'bb2.exe' | sort | xargs sha1sum) > "$OUT/$name.sha1"
}
run serial1 make check
run serial2 make check
run par8 make -j8 check
run serial3 make check
run par16 make -j16 check
for r in serial2 par8 serial3 par16; do
  echo "== serial1 vs $r" >> "$OUT/summary.txt"
  diff "$OUT/serial1.sha1" "$OUT/$r.sha1" >> "$OUT/summary.txt" && echo "identical" >> "$OUT/summary.txt"
done
