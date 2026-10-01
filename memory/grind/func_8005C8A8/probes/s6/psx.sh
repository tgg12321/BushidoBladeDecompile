#!/bin/bash
# psx.sh <name> : splice tmp/c8a8C/<name>.c into a copy of src/text1b.c (fix1 game.h via tmp/c8a8C/inc),
# compile with our cc1 and with the original cc1psx (calibration only), print frame + sp offsets of each.
cd "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
n="$1"
out=tmp/c8a8C/psx_$n; rm -rf "$out"; mkdir -p "$out"
python3 - tmp/c8a8C/$n.c "$out/tu.c" <<'EOF'
import sys
src = open('src/text1b.c').read()
line = 'INCLUDE_ASM("asm/funcs", func_8005C8A8);'
assert src.count(line) == 1
open(sys.argv[2], 'w').write(src.replace(line, open(sys.argv[1]).read()))
EOF
mipsel-linux-gnu-cpp -Itmp/c8a8C/inc/include -Iinclude -Isrc -undef -Wall -lang-c -fno-builtin -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL -D_LANGUAGE_C -DLANGUAGE_C "$out/tu.c" > "$out/tu.i" 2>/dev/null
tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float "$out/tu.i" -o "$out/ours.s" 2> "$out/ours.err"
echo "cc1 exit $?"
tools/cc1psx_wrapper.sh -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -msoft-float -w < "$out/tu.i" > "$out/psx.s" 2> "$out/psx.err"
echo "cc1psx exit $?"
python3 - "$out" <<'EOF'
import re, sys
d = sys.argv[1]
for name in ['psx.s', 'ours.s']:
    t = open(d + '/' + name, errors='replace').read()
    m = re.search(r'\nfunc_8005C8A8:\n', t)
    if not m:
        print(name, 'NO FUNC'); continue
    e = t.find('.end\tfunc_8005C8A8', m.start())
    body = t[m.start():e]
    open(d + '/' + name + '.fn', 'w').write(body)
    fr = re.search(r'\.frame\s+[^\n]*', body)
    slots = sorted(set(re.findall(r'(\d+)\(\$sp\)', body)), key=int)
    print(name, fr.group(0) if fr else '?', 'lines', body.count('\n'))
    print('  sp offsets:', ' '.join(slots))
    print('  1264 refs:', [l.strip() for l in body.splitlines() if '1264' in l or '0x4f0' in l.lower()])
EOF
