#!/bin/bash
# usage: pp.sh cand.c out.i   (head.h + candidate -> preprocessed TU)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cat tmp/f8b488s2/head.h "$1" | mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin \
  -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx \
  -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C 2>/dev/null > "$2"
