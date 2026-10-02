#!/bin/bash
# Ruling 11 (D)(1) dumps for func_8005490C: per spelling, the TU copy (tmp/func_8005490C/x_m_<body>/text1b.c,
# built by mk.py: minimal (A) + body) is preprocessed with the build cpp flags plus -DPERMUTER (INCLUDE_ASM
# bodies dropped), every other function body stripped (tools/decomp-permuter/strip_other_fns.py), then compiled
# with the instrumented tools/gcc-2.7.2/cc1 at the build flags with -dr -ds -dc -df -dl -dg.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile" && source .venv/bin/activate
R=$PWD
D=tmp/func_8005490C/dumps
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -P -DPERMUTER -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
FLAGS="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
for spec in "$@"; do
  t=${spec%%:*}; b=${spec#*:}
  mkdir -p $D/$t
  $CPP tmp/func_8005490C/x_m_$b/text1b.c > $D/$t/tu.i
  python3 tools/decomp-permuter/strip_other_fns.py $D/$t/tu.i func_8005490C
  (cd $D/$t && "$R/tools/gcc-2.7.2/cc1" $FLAGS -dr -ds -dc -df -dl -dg -o f.s tu.i && "$R/tools/gcc-2.7.2/build/cc1" $FLAGS -o f_build.s tu.i && cmp f.s f_build.s && echo "$t ($b): instrumented .s == build .s")
done
