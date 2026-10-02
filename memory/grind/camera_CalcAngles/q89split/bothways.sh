#!/bin/bash
# bothways.sh <head.c> : compile the Q89 head through the per-file pipeline at cc1 -G0 and -G8; compare every
# function's disassembly + relocations (objdump -dr) and every data section; print differing functions.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
SRC=$1; O=tmp/cam/bw/obj; mkdir -p $O
MF="--expand-div --aspsx-version=2.34 --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section -G8"
CPPF="-Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C"
for G in 0 8; do
  set -o pipefail
  mipsel-linux-gnu-cpp $CPPF "$SRC" | tools/gcc-2.7.2/build/cc1 -O2 -G$G -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
   | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py $MF | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
   | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $O/g$G.o 2>/dev/null || { echo FAIL G$G; exit 1; }
  mipsel-linux-gnu-objdump -dr --no-show-raw-insn $O/g$G.o > $O/g$G.dis
  for s in .rodata .data .sdata .sbss .bss; do mipsel-linux-gnu-objdump -s -j $s $O/g$G.o 2>/dev/null | tail -n +5; done > $O/g$G.data
  mipsel-linux-gnu-objdump -r -j .rodata -j .sdata -j .data $O/g$G.o 2>/dev/null | tail -n +4 > $O/g$G.datarel
  mipsel-linux-gnu-nm -n $O/g$G.o | awk '{print $NF, $(NF-1)}' > $O/g$G.syms
done
python3 memory/grind/camera_CalcAngles/q89split/cmp.py $O/g0.dis $O/g8.dis
cmp -s $O/g0.data $O/g8.data && echo "data sections: identical" || echo "data sections: DIFFER"
cmp -s $O/g0.datarel $O/g8.datarel && echo "data relocations: identical" || echo "data relocations: DIFFER"
