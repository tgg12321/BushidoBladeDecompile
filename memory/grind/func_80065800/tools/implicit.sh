#!/bin/bash
# implicit.sh <tree-root> <file.c>...: the build's cpp | cc1 (build flags, but -Wimplicit in place
# of -w) on each file; prints "<file> <name>" for every implicit function declaration cc1 reports.
# Used to compare a TU move's implicit-declaration sets before and after (rodata-align doc §7/§9).
R="$1"; shift
cd "$R" || exit 1
for f in "$@"; do
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ \
    -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C \
    -DLANGUAGE_C "$f" 2>/dev/null \
  | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls \
    -fno-builtin -Wimplicit -mel -msoft-float -o /dev/null 2>&1 \
  | grep -o "implicit declaration of function \`[A-Za-z_0-9]*'" \
  | sed "s/.*\`\(.*\)'/\1/" | sort -u | sed "s#^#$(basename "$f") #"
done
