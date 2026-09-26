#!/bin/bash
# psx.sh <candidate.c> <tag> : splice candidate into a copy of src/text1b.c, compile with
# BOTH cc1psx (original PsyQ) and our build/cc1 through the real pipeline, objdump
# func_8005D814 from each, and report the diff line count vs target (cmp.py full).
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
C=$1; T=$2; D=tmp/func_8005D814/psx_$T; mkdir -p $D
python3 - "$C" "$D/text1b.c" <<'PY'
import sys
cand=open(sys.argv[1]).read()
s=open('src/text1b.c').read()
old='INCLUDE_ASM("asm/funcs", func_8005D814);\n'
assert s.count(old)==1
open(sys.argv[2],'w').write(s.replace(old,cand))
PY
mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C $D/text1b.c > $D/t.i 2>/dev/null
FL="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
MX="--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt"
for which in psx ours; do
  if [ $which = psx ]; then tools/cc1psx_wrapper.sh $FL < $D/t.i > $D/$which.s0 2>$D/$which.err; else tools/gcc-2.7.2/build/cc1 $FL < $D/t.i > $D/$which.s0 2>$D/$which.err; fi
  python3 tools/prologue_fix.py < $D/$which.s0 | python3 tools/maspsx/maspsx.py $MX | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $D/$which.o 2>>$D/$which.err
  python3 tmp/func_8005D814/dis.py $D/$which.o >/dev/null && cp tmp/func_8005D814/ours.txt tmp/func_8005D814/ours_$T_$which.txt
  echo "== $T $which: $(python3 tmp/func_8005D814/cmp.py full | tail -1)"
done
