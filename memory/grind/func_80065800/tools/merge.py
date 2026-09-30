"""Aggregate merge of the per-mode tables at 0x800F0BA8 (s16[18]) and 0x800F0CA0
(Unk800F0C10Record[18]) in src/text1b_tu1c.c + include/game.h.
usage: merge.py [--src PATH]   (default src/text1b_tu1c.c + include/game.h; writes in place)"""
import re, sys
from pathlib import Path

src = Path(sys.argv[sys.argv.index('--src') + 1]) if '--src' in sys.argv else Path('src/text1b_tu1c.c')
t = src.read_bytes().decode('utf-8')
assert '\r' not in t


def in_tmr(a):
    return 0x800F0BA8 <= a < 0x800F0BCC and a % 2 == 0


def in_rec(a):
    return 0x800F0CA0 <= a < 0x800F0D78 and a % 4 == 0


def tmr(a):
    return f'@@TMR@@[{(a - 0x800F0BA8) // 2}]'


def rec(a):
    off = a - 0x800F0CA0
    return f'@@POS@@[{off // 12}].unk{off % 12:X}'


# 1. drop the per-word extern declarations (whole lines)
out, dropped = [], 0
for line in t.split('\n'):
    m = re.match(r'^\s*extern (?:s16|u16|s32|u32) D_(800F0[0-9A-F]{3});\s*$', line)
    if m and (in_tmr(int(m.group(1), 16)) or in_rec(int(m.group(1), 16))):
        dropped += 1
        continue
    out.append(line)
t = '\n'.join(out)

# 2. puns on timer entries
t = re.sub(r'\(u16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: '&' + tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)
t = re.sub(r'\*\(s16 \*\)&D_(800F0[0-9A-F]{3})\b',
           lambda m: tmr(int(m.group(1), 16)) if in_tmr(int(m.group(1), 16)) else m.group(0), t)

# 3. remaining references
def repl(m):
    a = int(m.group(1), 16)
    if in_tmr(a):
        return tmr(a)
    if in_rec(a):
        return rec(a)
    return m.group(0)
t = re.sub(r'\bD_(800F0[0-9A-F]{3})\b', repl, t)

# 4. u16 pointer locals bound to a timer entry become s16 pointers
t = re.sub(r'\bu16 \*(\w+) = &@@TMR@@\[', r's16 *\1 = &@@TMR@@[', t)
old = '    u16 *v1;\n    s32 v0;\n    func_80065800(0xC);\n    v1 = &@@TMR@@[12];'
assert t.count(old) == 1
t = t.replace(old, old.replace('u16 *v1;', 's16 *v1;'))

t = t.replace('@@TMR@@', 'D_800F0BA8').replace('@@POS@@', 'D_800F0CA0')
src.write_bytes(t.encode('utf-8'))
print('dropped externs:', dropped)
left = [l for l in t.split('\n') if re.search(r'u16 \*\w+ = &D_800F0BA8|\(u16 \*\)&D_800F0BA8|\(s16 \*\)&D_800F0BA8', l)]
print('remaining casts/u16 ptrs:', left)

if '--src' not in sys.argv:
    g = Path('include/game.h')
    h = g.read_bytes().decode('utf-8')
    anchor = 'extern Unk800F0C10Record D_800F0C10[4][3];\n'
    assert h.count(anchor) == 1
    h = h.replace(anchor, anchor + """
/* 0x800F0BA8: one s16 per effect mode (0..0x11), stepped each frame by that mode's wrapper
 * (func_800652F4..func_800657B0) and read by the shared draw routine func_80065800. Object
 * model: the original func_80065800 indexes it as base + mode*2 (and base + (mode-2)*2 for
 * modes 8/9). Replaces the splat per-word scalars D_800F0BA8..D_800F0BCA. */
extern s16 D_800F0BA8[18];

/* 0x800F0CA0: one 12-byte position record per effect mode, filled by the per-mode init
 * functions (func_80064E90..func_800657B0's siblings) and read by func_80065800 as
 * base + mode*12 + {0,4,8} (the same record shape as D_800F0C10's). Replaces the splat
 * per-word scalars D_800F0CA0..D_800F0D74. */
extern Unk800F0C10Record D_800F0CA0[18];
""")
    g.write_bytes(h.encode('utf-8'))
    print('header ok')
