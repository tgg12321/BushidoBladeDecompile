#!/bin/bash
# g8cmp.sh: compile tmp/camera_CalcAngles/text1b_cand.c (src/text1b.c + candidate spliced) through the build's
# text1b pipeline with cc1 -G0 and -G8; compare every function body by symbol (order-free) and the data sections.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
O=/tmp/camG8; mkdir -p $O
SRC=${1:-tmp/camera_CalcAngles/text1b_cand.c}
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
for g in 0 8; do
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $SRC 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G$g -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float > $O/g$g.cc1.s
  python3 tools/prologue_fix.py < $O/g$g.cc1.s | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt > $O/g$g.s
  mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $O/g$g.o $O/g$g.s
  mipsel-linux-gnu-objcopy --set-section-alignment .rodata=4 $O/g$g.o
  mipsel-linux-gnu-objdump -dr --no-show-raw-insn $O/g$g.o | sed 's/^ *[0-9a-f]*:\t//' > $O/g$g.dis
  mipsel-linux-gnu-objdump -h $O/g$g.o | grep -E "^\s+[0-9]+ \." > $O/g$g.h
done
python3 memory/grind/camera_CalcAngles/g0proof/g8cmp.py $O
