#!/bin/bash
# frames.sh: for each final probe, splice it over func_8005E54C's definition in a copy of src/text1b.c, compile
# with the Makefile's cpp + cc1 flags, and print cc1's .frame line; also sha1 of the function's cc1 listing.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=memory/grind/func_8005E54C/final_probes
W=tmp/func_8005E54C/frames
mkdir -p $W
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
CC1="tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
echo "cpp: $CPP"
echo "cc1: $CC1"
for p in "$@"; do
  python3 - $D/$p.c $W/$p.c <<'EOF'
import sys
src = open('src/text1b.c').read()
a = src.index('s32 func_8005E54C(u32 arg0, s32 arg1, s32 arg2) {')
b = src.index('\n}\n', a) + 3
open(sys.argv[2], 'w').write(src[:a] + open(sys.argv[1]).read() + src[b:])
EOF
  $CPP $W/$p.c 2>/dev/null | $CC1 -o $W/$p.s
  awk '/^func_8005E54C:/{f=1} f{print} f&&/\.end\tfunc_8005E54C/{exit}' $W/$p.s > $W/$p.f.s
  echo "$p: $(grep '\.frame' $W/$p.f.s | sed 's/.*# //')  listing-sha1 $(sha1sum < $W/$p.f.s | cut -c1-12)"
done
