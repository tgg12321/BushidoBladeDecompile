#!/bin/bash
# usage: dump.sh <candidate.c> [extra env assignments...]
# Splices the candidate into a COPY of src/code6cac_b.c, runs the instrumented
# cc1 with -da and BB2_ALLOC_DEBUG, and leaves dumps in tmp/func_80030D7C/dump/.
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
D=tmp/func_80030D7C/dump
mkdir -p $D
rm -f $D/t.i.*
python3 - "$1" <<'EOF'
import sys
src = open('src/code6cac_b.c').read()
cand = open(sys.argv[1]).read()
line = 'INCLUDE_ASM("asm/funcs", func_80030D7C);'
assert src.count(line) == 1
open('tmp/func_80030D7C/dump/t.c', 'w').write(src.replace(line, cand))
EOF
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/t.c > $D/t.i 2>/dev/null
shift
env BB2_ALLOC_DEBUG=1 "$@" tools/gcc-2.7.2/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -da $D/t.i -o $D/t.s 2> $D/stderr.txt || true
grep "func=func_80030D7C" $D/stderr.txt > $D/alloc.txt || true
wc -l $D/alloc.txt
