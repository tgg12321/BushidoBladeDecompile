#!/bin/bash
# (a2) calibration on the exact staged body: original PsyQ cc1psx (-G8) on the landed code6cac_b5.c
# (CdState members) and on the separate-variable variant (v/sep, same body, E9C/EA4 as externs).
set -eo pipefail
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"; source .venv/bin/activate
O=memory/grind/func_80036140/q62/psx; mkdir -p $O
python3 tmp/func_80036140/mk.py sep tmp/func_80036140/body_q62.c >/dev/null
DEFS="-Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
mipsel-linux-gnu-cpp -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $DEFS src/code6cac_b5.c > tmp/func_80036140/psx_rec.i
mipsel-linux-gnu-cpp -Itmp/func_80036140/v/sep/inc -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin $DEFS tmp/func_80036140/v/sep/code6cac_b5.c > tmp/func_80036140/psx_sep.i
F="-O2 -G8 -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w"
bash tools/cc1psx_wrapper.sh $F < tmp/func_80036140/psx_rec.i > $O/staged_record_member.cc1psx-G8.s
bash tools/cc1psx_wrapper.sh $F < tmp/func_80036140/psx_sep.i > $O/separate_vars.cc1psx-G8.s
echo "cc1psx: bash tools/cc1psx_wrapper.sh $F < <preprocessed>; staged = src/code6cac_b5.c as landed; sep = tmp mk.py sep variant (q62/tools/mk.py) of the same body" > $O/CMD
for f in $O/*.s; do echo "== $f"; grep -nE "D_80101E9C|D_80101EA4|D_80101E58\+(68|76)" $f; done
