#!/bin/bash
# cmpall.sh <G> <text1b_variant.c> [incdir-override] : compile a full text1b variant (cc1 -G<G>, maspsx -G8)
# and report every function whose disassembly differs from build/src/text1b.o (order-free), plus function order.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
G=$1; SRC=$2; INC=${3:+-I$3}
O=/tmp/camT_$$; mkdir -p $O
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
mipsel-linux-gnu-cpp $INC -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $SRC 2>/dev/null \
 | tools/gcc-2.7.2/build/cc1 -O2 -G$G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float > $O/t.cc1.s || { echo CC1 FAILED; exit 1; }
python3 tools/prologue_fix.py < $O/t.cc1.s | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > $O/t.s
mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $O/t.o $O/t.s 2>/dev/null || { echo AS FAILED; exit 1; }
mipsel-linux-gnu-objdump -dr --no-show-raw-insn $O/t.o > $O/t.dis
mipsel-linux-gnu-objdump -dr --no-show-raw-insn build/src/text1b.o > $O/ref.dis
python3 memory/grind/camera_CalcAngles/types/cmpall.py $O/ref.dis $O/t.dis "${FUNCS:-}"
rm -rf $O
