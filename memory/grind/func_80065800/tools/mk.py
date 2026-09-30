"""Scratch copy of src/text1b_tu1c.c with land.py's step-1 edits applied (aggregate merge +
func_80065680 respelling) and the candidate substituted. Writes tmp/f65800/re/tu1c.c only.
The two array declarations are added TU-locally (measurement only; the landing puts them in
include/game.h).  usage (WSL, repo root): python3 tmp/f65800/re/mk.py [candidate.c]"""
import re
import sys
from pathlib import Path

NL = chr(10)
cand = sys.argv[1] if len(sys.argv) > 1 else 'memory/grind/func_80065800/candidate.c'


def in_tmr(a):
    return 0x800F0BA8 <= a < 0x800F0BCC and a % 2 == 0


def in_rec(a):
    return 0x800F0CA0 <= a < 0x800F0D78 and a % 4 == 0


def tmr(a):
    return '@@TMR@@[%d]' % ((a - 0x800F0BA8) // 2)


def rec(a):
    off = a - 0x800F0CA0
    return '@@POS@@[%d].unk%X' % (off // 12, off % 12)


def sub1(t, a, b):
    assert t.count(a) == 1, a[:100]
    return t.replace(a, b)


t = Path('src/text1b_tu1c.c').read_bytes().decode('utf-8')
assert '\r' not in t
out, dropped = [], 0
for line in t.split(NL):
    m = re.match(r'^\s*extern (?:s16|u16|s32|u32) D_(800F0[0-9A-F]{3});\s*$', line)
    if m and (in_tmr(int(m.group(1), 16)) or in_rec(int(m.group(1), 16))):
        dropped += 1
        continue
    out.append(line)
t = NL.join(out)
t = re.sub(r'\(u16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: '&' + tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)
t = re.sub(r'\*\(s16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)


def repl(m):
    a = int(m.group(1), 16)
    if in_tmr(a):
        return tmr(a)
    if in_rec(a):
        return rec(a)
    return m.group(0)


t = re.sub(r'\bD_(800F0[0-9A-F]{3})\b', repl, t)
t = re.sub(r'\bu16 \*(\w+) = &@@TMR@@\[', r's16 *\1 = &@@TMR@@[', t)
t = sub1(t, '    u16 *v1;\n    s32 v0;\n    func_80065800(0xC);\n    v1 = &@@TMR@@[12];\n    v0 = *v1 + 1;\n    *v1 = v0;\n',
         '    s16 *v1;\n    s32 v0;\n    func_80065800(0xC);\n    v1 = &@@TMR@@[12];\n    *v1 = *v1 + 1;\n')
t = sub1(t, 'if ((s16)@@TMR@@[14] >= 11)', 'if (@@TMR@@[14] >= 11)')
t = t.replace('@@TMR@@', 'D_800F0BA8').replace('@@POS@@', 'D_800F0CA0')
t = sub1(t, '#include "code6cac.h"\n', '#include "code6cac.h"\nextern s16 D_800F0BA8[18];\nextern Unk800F0C10Record D_800F0CA0[18];\n')
t = sub1(t, 'INCLUDE_ASM("asm/funcs", func_80065800);\n', Path(cand).read_bytes().decode('utf-8'))
Path('tmp/f65800/re/tu1c.c').write_bytes(t.encode('utf-8'))
print('dropped externs', dropped)
