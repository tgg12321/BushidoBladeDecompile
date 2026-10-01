#!/bin/bash
# run.sh <variant-dir>: splice + full text1b pipeline + standalone link compare with the oracle image
set -e
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
source .venv/bin/activate
V=$1; O=$V/o; H=tmp/func_80058580/h
python3 $H/splice.py $V $O >/dev/null
mipsel-linux-gnu-cpp -I$O/inc -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C < $O/text1b.c > $O/t.i
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float $O/t.i -o $O/t.s 2>&1 | grep -v "^$" | head -20 || true
python3 tools/prologue_fix.py < $O/t.s | python3 tools/maspsx/maspsx.py --expand-div --aspsx-version=2.34 --sdata-syms=sdata_syms.txt --sdata-funcs=sdata_funcs.txt --sdata-exclude=sdata_exclude.txt --expand-lb --expand-lb-funcs=expand_lb_funcs.txt --multu-funcs=multu_funcs.txt --expand-dest-funcs=expand_dest_funcs.txt --prefill-label-funcs=maspsx_prefill_label_funcs.txt --comm-syms=maspsx_comm_syms.txt --use-comm-section | python3 tools/multu_pad.py --funcs multu_pad_funcs.txt | mipsel-linux-gnu-as -Iinclude -march=r3000 -mtune=r3000 -no-pad-sections -O1 -G0 -o $O/text1b.o
mipsel-linux-gnu-objcopy --set-section-alignment .rodata=4 $O/text1b.o
python3 $H/linkcheck.py $O/text1b.o
python3 - $O/text1b.o <<'PY'
import sys; sys.path.insert(0,'.')
from engine import score
o=sys.argv[1]
ref='build/src/text1b.o'
bad=[]
for f in sorted(score._o_func_table(ref).keys()):
    try:
        r=score.score_func(o, ref, f)
    except KeyError:
        bad.append((f,'MISSING')); continue
    if r.get('score') or r.get('build_insns')!=r.get('target_insns'):
        bad.append((f, r.get('score'), r.get('build_insns'), r.get('target_insns')))
print('func scores nonzero:', bad if bad else 'none')
PY
