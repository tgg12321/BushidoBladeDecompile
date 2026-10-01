#!/bin/bash
# fullobj.sh <candidate.c>: full Makefile pipeline for text1b with the candidate spliced (jtbl arrays removed)
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tmp/func_80058580/full; mkdir -p $D
python3 memory/grind/func_80058580/probes/s5/splice.py $1 $D/text1b.c
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C < $D/text1b.c > $D/t.i
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $D/t.i -o $D/t.s
python3 tools/prologue_fix.py < $D/t.s | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $D/text1b.o
mipsel-linux-gnu-objcopy $(grep '^RODATA_OBJ_ALIGN' Makefile | sed 's/.*:= *//') $D/text1b.o
python3 memory/grind/func_80058580/probes/s5/linkcheck.py $D/text1b.o
