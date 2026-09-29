#!/bin/bash
# fast.sh <tag>... : gen+splice+full pipeline, diff vs target; prints tag, #diff lines, insn counts
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate 2>/dev/null
D=tmp/func_800187F4
[ -f $D/tgt.s ] || { echo "no tgt.s"; exit 1; }
for T in "$@"; do
  W=$D/fast_$T; mkdir -p $W
  python3 $D/gen.py $D/$T.c $D/out_$T.c ${GENMODE:-} || continue
  python3 - "$T" <<'PY'
import sys
t=sys.argv[1]
src=open('tmp/f187/code6cac_head.c').read()
cand=open(f'tmp/func_800187F4/out_{t}.c').read()
key='INCLUDE_ASM("asm/funcs", func_800187F4);'
S16='void func_800187F4(s16 *arg0, s32 *arg1);'; S32='void func_800187F4(s32 arg0, s32 *arg1);'
src=src.replace(S32,S16) if 'func_800187F4(s16 *arg0' in cand else src.replace(S16,S32)
open(f'tmp/func_800187F4/fast_{t}/code6cac.c','w').write(src.replace(key,cand))
PY
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $W/code6cac.c 2>/dev/null \
   | tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float \
   | python3 tools/prologue_fix.py \
   | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt \
   | sed "s/\.align\t3/.align\t2/" \
   | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt \
   | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $W/f.o 2>$W/as.err || { echo "$T BUILD FAIL"; head -5 $W/as.err; continue; }
  [ -s $W/cc1.err ] && { echo "$T CC1 ERR"; head -3 $W/cc1.err; continue; }
  mipsel-linux-gnu-objdump -d -z --no-show-raw-insn $W/f.o | awk '/<func_800187F4>:/{f=1;next} f&&/^$/{exit} f' | sed -e 's/^ *\([0-9a-f]*\):\t/\1 /' -e 's/\t/ /g' -e 's/<[^>]*>//' > $W/ours.s
  python3 $D/cmp.py $D/tgt.s $W/ours.s $W/sbs.txt "$T"
done
