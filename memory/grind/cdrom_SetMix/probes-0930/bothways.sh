#!/bin/bash
# Both-ways byte-neutrality check of the proposed COMMON gate: every src/*.c is compiled with the
# Makefile's exact per-file recipe (taken from `make -n -B`, nothing executed by make) twice --
# (A) stock tools/maspsx, (B) tmp copy with the gate + tmp/research36140/maspsx_comm_syms.txt --
# into tmp/cdrom_SetMix/bw/{A,B}/<stem>.o, then compares the objects byte for byte.
# usage (WSL, repo root, venv active): bash tmp/research36140/bothways.sh
set -o pipefail
OUT=tmp/cdrom_SetMix/bw
rm -rf "$OUT"; mkdir -p "$OUT/A" "$OUT/B"
same=0; diff=0; fail=0
for c in src/*.c; do
  stem=$(basename "$c" .c)
  cmd=$(make -n -B "build/src/$stem.o" 2>/dev/null | grep -F "maspsx.py" | grep -F -- "-o build/src/$stem.o" | tail -1)
  if [ -z "$cmd" ]; then echo "!! no recipe for $stem"; fail=$((fail+1)); continue; fi
  a=${cmd//"-o build/src/$stem.o"/"-o $OUT/A/$stem.o"}
  b=${cmd//"-o build/src/$stem.o"/"-o $OUT/B/$stem.o"}
  b=${b//"python3 tools/maspsx/maspsx.py"/"python3 tmp/cdrom_SetMix/maspsx_scratch/maspsx.py --use-comm-section"}
  if ! bash -o pipefail -c "$a" 2>"$OUT/A/$stem.err"; then echo "!! A failed $stem"; fail=$((fail+1)); continue; fi
  if ! bash -o pipefail -c "$b" 2>"$OUT/B/$stem.err"; then echo "!! B failed $stem"; fail=$((fail+1)); continue; fi
  if cmp -s "$OUT/A/$stem.o" "$OUT/B/$stem.o"; then same=$((same+1)); else diff=$((diff+1)); echo "DIFF $stem"; fi
done
echo "files: identical=$same differ=$diff failed=$fail"
