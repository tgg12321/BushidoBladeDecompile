#!/bin/bash
# (D)(1) dump excerpts for the Ruling 11 record: per-value spelling (F3) vs reuse (X).
# usage: dproof.sh <out.txt>
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
OUT="$1"
D=tmp/func_80030D7C/dump
: > "$OUT"
run() {  # $1 label, $2 candidate, $3.. pseudos
  local label="$1" cand="$2"; shift 2
  echo "=== $label: $cand ===" >> "$OUT"
  echo "command: tmp/func_80030D7C/dump.sh $cand  (cpp | tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da, BB2_ALLOC_DEBUG=1)" >> "$OUT"
  bash tmp/func_80030D7C/dump.sh "$cand" > /dev/null
  awk '/^;; Function func_80030D7C/{f=1} /^;; Function func_80031890/{f=0} f' $D/t.i.lreg > $D/lreg_$label.txt
  for p in "$@"; do
    grep "^Register $p " $D/lreg_$label.txt >> "$OUT"
    grep "pseudo=$p " $D/alloc.txt >> "$OUT"
    bash tmp/func_80030D7C/dump.sh "$cand" BB2_FINDREG_DEBUG=$p > /dev/null
    grep "FINDREGDBG" $D/stderr.txt | grep -A11 "func=func_80030D7C pseudo=$p " | head -12 >> "$OUT"
  done
}
run perval tmp/func_80030D7C/PV.c 75 76 78 79
run reuse tmp/func_80030D7C/L1.c 75 77
echo done
