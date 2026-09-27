#!/bin/bash
# usage (WSL, repo root): bash tmp/func_8001A820/micro.sh <file.c>
set -e
f="$1"; b="${f%.c}"
flags="-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float"
mipsel-linux-gnu-cpp -undef -lang-c "$f" > "$b.i" 2>/dev/null || cpp -P "$f" > "$b.i"
tools/gcc-2.7.2/build/cc1 $flags "$b.i" -o "$b.ours.s"
tools/cc1psx_wrapper.sh $flags < "$b.i" > "$b.psx.s"
python3 - "$b" <<'EOF'
import re, sys
b = sys.argv[1]
def funcs(p):
    t = open(p, errors='replace').read()
    return dict(re.findall(r'\n(t\w+):\n(.*?)\.end\s+t\w+', t, re.S))
o, p = funcs(b + '.ours.s'), funcs(b + '.psx.s')
for k in sorted(o):
    oo = [l.strip() for l in o[k].splitlines() if l.strip() and not l.strip().startswith(('.', '#'))]
    pp = [l.strip() for l in p.get(k, '').splitlines() if l.strip() and not l.strip().startswith(('.', '#'))]
    print('==', k)
    print('  ours:', ' | '.join(x for x in oo if re.match(r'(lh|lhu|sll|sra|sh)\b', x)))
    print('  psx :', ' | '.join(x for x in pp if re.match(r'(lh|lhu|sll|sra|sh)\b', x)))
EOF
