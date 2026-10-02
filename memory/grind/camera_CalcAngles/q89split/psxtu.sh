#!/bin/bash
# psxtu.sh <head.c> : calibration only (Q89 condition (ii)) - original cc1psx on the head TU's preprocessed
# source at -G0 and -G8; print camera_CalcAngles's code from each and every gp-relative (%gp_rel / sym-only
# small-data) reference to D_800A33C8.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
CPPF="-Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
O=tmp/cam/bw/psx; mkdir -p $O
mipsel-linux-gnu-cpp $CPPF "$1" > $O/head.i 2>/dev/null
for G in 0 8; do
  bash tools/cc1psx_wrapper.sh -O2 -G$G -funsigned-char -mcpu=3000 -mips1 -msoft-float -w < $O/head.i > $O/head_g$G.s || echo "cc1psx G$G FAILED"
  echo "=== cc1psx -G$G: camera_CalcAngles"
  awk '/^camera_CalcAngles:/{p=1} p{print} p&&/\.end\s+camera_CalcAngles/{exit}' $O/head_g$G.s | grep -vE '^\s*#|^$|^\s*\.(frame|mask|fmask|loc)'
done
