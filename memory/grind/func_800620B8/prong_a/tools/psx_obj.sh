#!/bin/bash
# usage: psx_obj.sh <variant-name>: cc1psx output of d/<name>/t.i through the build's post-cc1 pipeline
# (prologue_fix | maspsx | align fix | multu_pad | as), then objdump func_800620B8 -> d/<name>/psx_obj.txt,
# and the same for the target (asm/funcs/func_800620B8.s instructions) -> target.txt; diff -> psx_vs_target.diff
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
D=tmp/func_800620B8/s3/d/$1
FL="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
MF="--expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt"
echo "cc1psx->obj: bash tools/cc1psx_wrapper.sh $FL < t.i | prologue_fix | maspsx $MF | align-fix | multu_pad | as (build AS_FLAGS) -> psx.o; objdump -dr -M no-aliases -> psx_obj.txt; cmp.py -> focus.txt, psx_vs_target.diff" >> $D/cmd.txt
bash tools/cc1psx_wrapper.sh $FL < $D/t.i 2>/dev/null | python3 tools/prologue_fix.py | python3 tools/maspsx/maspsx.py $MF | sed "s/\.align\t3/.align\t2/" | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $D/psx.o
mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases $D/psx.o | awk '/<func_800620B8>:/{f=1;next} f&&/^$/{exit} f' | sed -E 's/^ *[0-9a-f]+:\t//' > $D/psx_obj.txt
python3 - $D <<'PY'
import re,sys
D=sys.argv[1]
out=[]
for l in open('asm/funcs/func_800620B8.s'):
    m=re.match(r'\s*/\* \w+ \w+ \w+ \*/\s+(\S+)\s*(.*)$', l)
    if m: out.append((m.group(1)+' '+m.group(2)).strip())
open(D+'/target.txt','w').write('\n'.join(out)+'\n')
PY
echo "psx insns: $(grep -c . $D/psx_obj.txt)  target insns: $(grep -c . $D/target.txt)"
