#!/bin/bash
# Calibration only: compile the landed code6cac_b3.c with the ORIGINAL PsyQ cc1psx at -G8 and -G0
# and compare the cursor / flags / walker addressing against our cc1.
cd "/mnt/c/Users/Trenton/desktop/Bushido Blade 2 Decompile"
D=tmp/func_80034708/psx; mkdir -p $D
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C src/code6cac_b3.c > $D/b3.i 2>/dev/null
for G in -G8 -G0; do
  bash tools/cc1psx_wrapper.sh -O2 $G -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w < $D/b3.i > $D/psx$G.s 2>$D/psx$G.err || echo "cc1psx $G failed rc=$?"
  tools/gcc-2.7.2/build/cc1 -O2 $G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float < $D/b3.i > $D/ours$G.s 2>/dev/null
  echo "== $G  psx: lines=$(wc -l < $D/psx$G.s) ours: lines=$(wc -l < $D/ours$G.s)"
  for f in psx ours; do
    printf '  %-5s cursor direct=%s  cursor-reg-lh=%s  frame=%s\n' $f \
      "$(grep -cE 'lh\s+\$[0-9]+,D_800A3174' $D/$f$G.s)" \
      "$(grep -cE 'lh\s+\$[0-9]+,[0-9]+\(\$1[6-9]\)|lh\s+\$[0-9]+,[0-9]+\(\$2[0-3]\)' $D/$f$G.s)" \
      "$(grep -m1 '\.frame' $D/$f$G.s | sed 's/\s\+/ /g')"
  done
  diff <(grep -vE '^\s*(#|\.)' $D/psx$G.s | sed 's/\s\+/ /g') <(grep -vE '^\s*(#|\.)' $D/ours$G.s | sed 's/\s\+/ /g') | grep -c '^[<>]' | sed 's/^/  psx-vs-ours differing lines: /'
done
