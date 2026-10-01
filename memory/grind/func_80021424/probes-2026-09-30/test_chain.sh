#!/bin/bash
# test_chain.sh : run land_all.sh on two scratch copies of main (as is, and with laneH's unk_6A edit applied first)
# and compare both with the measured tree tmp/prc/l1_tree.
set -e
cd "/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
for mode in plain lanehfirst; do
  T=tmp/prc/chain_$mode
  rm -rf $T && mkdir -p $T/src $T/include
  cp src/code6cac_tu2.c src/code6cac_c2.c $T/src/ && cp include/code6cac.h $T/include/ && cp undefined_syms_auto.txt $T/
  if [ $mode = lanehfirst ]; then
    T=$T python3 - <<'EOF'
import os
from pathlib import Path
p = Path(os.environ["T"]) / "include/code6cac.h"
s = p.read_bytes().decode()
a = "    u8  unk_60[0x72 - 0x60];\n"
assert s.count(a) == 1
p.write_bytes(s.replace(a, "    u8  unk_60[0x6A - 0x60];\n    u16 unk_6A;\n    u8  unk_6C[0x72 - 0x6C];\n").encode())
EOF
  fi
  L1_ROOT=$T bash tmp/prc/land_all.sh > /dev/null
  for f in include/code6cac.h src/code6cac_tu2.c src/code6cac_c2.c undefined_syms_auto.txt; do
    if diff -q $T/$f tmp/prc/l1_tree/$f > /dev/null; then echo "$mode $f identical"; else echo "$mode $f DIFFERS"; fi
  done
done
