#!/bin/bash
# g8fn.sh <G> <ffc_candidate.c|-> [cam_candidate.c|-] : splice candidates into a copy of src/text1b.c, compile
# the text1b pipeline with cc1 -G<G> (maspsx -G8 as built), and diff func_80048FFC / camera_CalcAngles
# against the reference object build/src/text1b.o (INCLUDE_ASM bytes).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
G=$1; FFC=$2; CAM=${3:--}; XF="${XFLAGS:-}"
O=/tmp/camG8fn_$$; mkdir -p $O
python3 tmp/camera_CalcAngles/g8/splice2.py "$FFC" "$CAM" $O/t.c || exit 1
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $O/t.c 2>/dev/null \
 | tools/gcc-2.7.2/build/cc1 -O2 -G$G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $XF > $O/t.cc1.s
python3 tools/prologue_fix.py < $O/t.cc1.s | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > $O/t.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $O/t.o $O/t.s || exit 1
mipsel-linux-gnu-objdump -dr --no-show-raw-insn $O/t.o > $O/t.dis
mipsel-linux-gnu-objdump -dr --no-show-raw-insn build/src/text1b.o > $O/ref.dis
python3 tmp/camera_CalcAngles/g8/fncmp.py $O/ref.dis $O/t.dis func_80048FFC camera_CalcAngles
cp $O/t.cc1.s tmp/camera_CalcAngles/g8/last.cc1.s
rm -rf $O
