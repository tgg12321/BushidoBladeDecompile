#!/bin/bash
# usage: pipe.sh <src.c> <maspsx_dir> <out.o>   (run from repo root, WSL, venv active)
# The sandbox's exact C pipeline (engine sandbox CMD line), with a selectable maspsx.
set -eo pipefail
SRC=$1; MAS=$2; OUT=$3
CFG=tmp/sandbox/func_8002DE20/cfg
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Isrc -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$SRC" 2>/dev/null \
 | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
 | PROLOGUE_CONFIG=$CFG/prologue_config.json DELAY_SLOT_RA_FUNCS=$CFG/delay_slot_ra_funcs.txt FRAME_FIX_FUNCS=$CFG/frame_fix_funcs.txt python3 tools/prologue_fix.py \
 | python3 "$MAS/maspsx.py" --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --expand-lb \
 | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
 | tee "${OUT%.o}.s" \
 | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o "$OUT"
mipsel-linux-gnu-objcopy -O binary -j .text "$OUT" "${OUT%.o}.text.bin"
sha1sum "${OUT%.o}.text.bin"
