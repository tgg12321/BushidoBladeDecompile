#!/bin/bash
# dump_sw.sh: .rtl dumps (cc1 -dr, the build's flags) of func_80065800's case 1/2/16/17 width
# scale, for the landed body (sw named) and rejected/sw-inline-14.c (scale written inline), in the
# spliced text1b_tu1c.c. Output: memory/grind/func_80065800/evidence/sw-rtl/{landed,inline}.rtl
# plus excerpt.txt (the insns from the rsin call's result to the mult feeding `>> 13`).
set -e
R="/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
cd "$R"
source .venv/bin/activate
O=memory/grind/func_80065800/evidence/sw-rtl
W=/tmp/laneC/swdump
rm -rf $W && mkdir -p $W $O
python3 - <<'PY'
import sys
sys.path.insert(0, '.')
from engine import inlineasm
t = open('src/text1b_tu1c.c', encoding='utf-8').read()
open('/tmp/laneC/swdump/landed.c', 'w', encoding='utf-8', newline='\n').write(t)
b = open('memory/grind/func_80065800/rejected/sw-inline-14.c', encoding='utf-8').read()
open('/tmp/laneC/swdump/inline.c', 'w', encoding='utf-8', newline='\n').write(
    inlineasm.substitute_body(t, 'func_80065800', b))
PY
for v in landed inline; do
  mipsel-linux-gnu-cpp -Iinclude -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ \
    -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C \
    -DLANGUAGE_C $W/$v.c > $W/$v.i
  (cd $W && "$R/tools/gcc-2.7.2/build/cc1" -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 \
    -mno-abicalls -fno-builtin -w -mel -msoft-float -dr $v.i -o $v.s)
  python3 - "$W/$v.i.rtl" "$O/$v.rtl" <<'PY'
import sys, re
t = open(sys.argv[1]).read()
a = t.index(';; Function func_80065800')
b = t.find(';; Function ', a + 10)
open(sys.argv[2], 'w').write(t[a:b if b > 0 else len(t)])
PY
done
python3 - "$O" <<'PY'
import sys, re
O = sys.argv[1]
out = []
for v in ('landed', 'inline'):
    t = open(O + '/' + v + '.rtl').read()
    # the width-scale sequence: from the call to rsin whose argument is (X<<11)/4551 - 0x400,
    # i.e. the first rsin call after the 4551 divide, to the first (ashiftrt ... 13)
    i = t.index('(const_int 4551')
    j = t.index('"rsin"', i)
    k = t.index('(const_int 13)', j)
    s = t.rfind('\n(insn', 0, j)
    e = t.find('\n\n', k)
    out.append('==== %s (.rtl, from the rsin call to the >> 13) ====\n%s\n' % (v, t[s:e]))
open(O + '/excerpt.txt', 'w').write('\n'.join(out))
print('\n'.join(out))
PY
