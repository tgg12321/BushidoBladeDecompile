#!/bin/bash
# fast.sh <cand> : splice, cpp, cc1 (no dumps); print function asm lines (instructions only) to <cand>.s
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
c=$1
python3 - "$c" "$c.tu.c" <<'PY'
import sys
src = open('src/code6cac_b_tu2.c').read()
line = 'INCLUDE_ASM("asm/funcs", func_8002AB08);'
open(sys.argv[2], 'w').write(src.replace(line, open(sys.argv[1]).read()))
PY
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$c.tu.c" 2>/dev/null | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -o "$c.full.s"
awk '/^func_8002AB08:/,/\.end\tfunc_8002AB08/' "$c.full.s" | grep -v "^\s*\.\|^\s*$\|^\$L\|^func" > "$c.s"
