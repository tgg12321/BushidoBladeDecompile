#!/bin/bash
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
F="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
CPP="mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
$CPP src/main/9F9C.c > tmp/s6/9f.i
tmp/cc1build/cc1 $F tmp/s6/9f.i -o tmp/s6/9f_new.s
tools/gcc-2.7.2/build/cc1.PRE-RECIPE-aa04d761 $F tmp/s6/9f.i -o tmp/s6/9f_ref.s
diff tmp/s6/9f_ref.s tmp/s6/9f_new.s | head -40
# which function
awk '/^\t\.ent/{f=$2} {print f"\t"$0}' tmp/s6/9f_new.s > tmp/s6/9f_new_f.s
diff <(awk '/^\t\.ent/{f=$2} {print f"\t"$0}' tmp/s6/9f_ref.s) tmp/s6/9f_new_f.s | grep '^[<>]' | cut -f1 | sort | uniq -c
