"""Stage 3 scratch integration: stage 2 (both aggregate merges) + full ings.c conversion
(func_800167EC with the record base taken first) + the TU split:
  code6cac_b.c        ... func_800344B4            (-G0, unchanged flags)
  code6cac_b3.c       func_80034708                 (-G8)
  code6cac_b3_post.c  func_80034F88 .. func_80035430 (-G0, moved verbatim)
and code6cac_b_rodata_pre.c without the two hand-transcribed jump tables."""
import os, re, subprocess
R = 'tmp/func_80034708/integ'
subprocess.run(['python3', 'tmp/func_80034708/mk_integ2.py'], check=True)

def rd(p):
    return open(p, encoding='utf-8').read()

def wr(p, s):
    open(p, 'w', encoding='utf-8', newline='\n').write(s)

# ---- ings.c: full conversion
s = rd('src/ings.c')
for d in ('extern u8 g_file_flags;\n', 'extern u32 g_file_disc_size;\n', 'extern u32 D_80106A5C;\n'):
    assert d in s
    s = s.replace(d, '')
old = '''    D_800A3710 = 0;
    g_file_flags = 0;
    p = (u8 *)&g_file_disc_size;
    *(s32 *)p = 0x7007;
    g_file_disc_type = 0;'''
new = '''    p = (u8 *)&D_80106A50;
    D_800A3710 = 0;
    D_80106A50.flags = 0;
    *(s32 *)p = 0x7007;
    D_80106A50.unk_04 = 0;'''
assert old in s
s = s.replace(old, new)
s = s.replace('    D_80106A5C = 0x6978;', '    D_80106A50.times[0].unk_4 = 0x6978;')
s = re.sub(r'\bg_file_flags\b', 'D_80106A50.flags', s)
assert not re.search(r'\bg_file_(flags|disc_size|disc_type)\b|D_80106A5C', s)
wr(R + '/src/ings.c', s)

# ---- (a4')(5)/(d): the two consumers that reached PracticeParams bytes through cross-member
# pointer arithmetic now name the members they write.
b = rd(R + '/src/code6cac_b.c')
old = '''    do {
        s8 *b = &D_80102778.unk_D;
        u8 *w = (u8 *)b - 9;
        s32 lv = (&D_8008D55C)[s[0]];
        w[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == new_var || (u32)(lv - 18) < 2 || (s8)lv == new_var2) {
            if (*b == 0) {
                w[i] = w[i] - 3;
            }
        }'''
new = '''    do {
        s32 lv = (&D_8008D55C)[s[0]];
        D_80102778.unk_4[i] = lv;
        if ((u32)(lv - 3) < 2 || (s8)lv == new_var || (u32)(lv - 18) < 2 || (s8)lv == new_var2) {
            if ((s8)D_80102778.unk_D == 0) {
                D_80102778.unk_4[i] = D_80102778.unk_4[i] - 3;
            }
        }'''
assert old in b, 'func_8003504C pun block not found'
b = b.replace(old, new)

# func_80035280: the flag reads name the flags member and the colour walker starts at the colour
# member (the old `f - 3` stepped from one member into another); the clock-record view is a
# FileTimeRec pointer instead of a byte re-view with record offsets.
F_COMMENT_START = '    /* FAKE: `f` is a redundant second handle to D_80106A73'
i0 = b.index(F_COMMENT_START)
i1 = b.index('    u8 *f;\n', i0) + len('    u8 *f;\n')
b = b[:i0] + b[i1:]
reps = [
    ('''    p = func_80077D00();
    i = 0;
    f = &D_80106A50.flags;
    src = f - 3;
    flags = p[8];
    flags0 = (flags & ~1) | (src[3] & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (src[3] & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (src[3] & 4);
    p[8] = flags2;
''', '''    p = func_80077D00();
    i = 0;
    flags = p[8];
    flags0 = (flags & ~1) | (D_80106A50.flags & 1);
    p[8] = flags0;
    flags1 = (flags0 & ~2) | (D_80106A50.flags & 2);
    p[8] = flags1;
    flags2 = (flags1 & ~4) | (D_80106A50.flags & 4);
    p[8] = flags2;
    src = D_80106A50.color;
'''),
    ('''    /* FAKE: typed re-view of the global D_80106A58 as the byte-strided base of
     * the three 8-byte clock records, hoisted above the loop rather than
     * respelled at each use;''', '''    /* FAKE: one pointer to the three 8-byte clock records (D_80106A50.times),
     * hoisted above the loop rather than indexing D_80106A50.times[i] at each
     * use;'''),
    ('    u8 *base;\n    s32 i;\n    s32 flags;\n', '    FileTimeRec *base;\n    s32 i;\n    s32 flags;\n'),
    ('    base = (u8 *)D_80106A50.times;\n', '    base = D_80106A50.times;\n'),
    ('        mn = *(s32 *)(base + i * 8 + 4) / 1800;\n', '        mn = base[i].unk_4 / 1800;\n'),
    ('        sc = (*(s32 *)(base + i * 8 + 4) / 30) % 60;\n', '        sc = (base[i].unk_4 / 30) % 60;\n'),
    ('        hs = (*(s32 *)(base + i * 8 + 4) % 30) * 100 / 30;\n', '        hs = (base[i].unk_4 % 30) * 100 / 30;\n'),
    ('        t = base[i * 8];\n', '        t = base[i].unk_0;\n'),
]
for o, n in reps:
    assert b.count(o) == 1, o[:60]
    b = b.replace(o, n)
# func_80033DF4-area flag word pointers: the s32 flags word is the record's first member
assert b.count('            flags = &D_80106A50;\n') == 2
b = b.replace('            flags = &D_80106A50;\n', '            flags = &D_80106A50.unk_00;\n')
wr(R + '/src/code6cac_b.c', b)
# func_8003B5A4 keeps its `chardata = &D_80102778.unk_4[1]; chardata[0] / chardata[2]`:
# with the pairs one u8[6] member, [1] and [3] of that array are in bounds (no cross-member step).
c = rd(R + '/src/code6cac_c_ab.c')
assert '    chardata = &D_80102778.unk_4[1];\n' in c and '                chardata[2] = p[1];\n' in c

subprocess.run(['python3', 'tmp/func_80034708/record_members.py'], check=True)

if os.environ.get('NOSPLIT'):
    print('stage3 (no split) ok')
    raise SystemExit(0)

# ---- split code6cac_b.c
b = rd(R + '/src/code6cac_b.c')
k = b.index('/* kengo:LOW  |  su_menu_vs/_DispSamnailWindow')
head, tail = b[:k], b[k:]
lines = tail.split('\n')
assert lines[0].startswith('/* kengo:LOW') and lines[1] == 'INCLUDE_ASM("asm/funcs", func_80034708);', lines[:2]
tail = '\n'.join(lines[2:])
# externs only the tail uses move with it
moved = []
for d in ('extern s32 *func_80077D00(void);\n', 'extern u8 D_801027A0;\n', 'extern u8 D_801027D8;\n'):
    assert d in head, d
    rest = head.replace(d, '')
    name = re.search(r'(\w+)\s*(\(|;)', d.split(' ', 2)[2]).group(1)
    if not re.search(r'\b%s\b' % name, rest):
        head = rest
    moved.append(d)
wr(R + '/src/code6cac_b.c', head.rstrip('\n') + '\n')

PRE = '''#include "common.h"
#include "include_asm.h"
#include "gpu.h"
#include "sound.h"
#include "game.h"
#include "system.h"
#include "gte.h"
#include "code6cac.h"
#include "bb2_const.h"
'''
B3_HDR = '''/* func_80034708's translation unit, compiled -G8 (Makefile GP_FILES). The
 * original compiler knew the 4-byte cursor array D_800A3174 was small data:
 * the target reads cursor[0] / cursor[1] straight off $gp while the loop walks
 * &cursor[i]. Under -G0 an array element's address stays in a register. The
 * same setting explains this function's strings: the <=8-byte ones sit in
 * .sdata at 0x800A3178.., the 9-byte ones in .rodata at 0x80010834. */
'''
POST_HDR = '''/* The code6cac_b.c functions that follow func_80034708, moved unchanged so
 * that func_80034708 can sit in its own -G8 unit (code6cac_b3.c) between them. */
'''
b3 = B3_HDR + PRE + '''
extern s32 rand(void);
extern void func_8005C650(s32, s32, s32);
extern void func_800344B4(void);

''' + rd('tmp/func_80034708/b3_body.c')
wr(R + '/src/code6cac_b3.c', b3)
post = POST_HDR + PRE + '\n' + ''.join(moved) + 'extern void func_800344B4(void);\n\n' + tail.lstrip('\n')
wr(R + '/src/code6cac_b3_post.c', post)

# ---- rodata_pre: drop the two jump tables (the compiler now emits them from code6cac_b3.c)
r = rd('src/code6cac_b_rodata_pre.c')
r2 = re.sub(r'\nconst u32 jtbl_8001086C\[12\] = \{.*?\};\n', '\n', r, flags=re.S)
r2 = re.sub(r'\nconst u32 jtbl_8001089C\[12\] = \{.*?\};\n', '\n', r2, flags=re.S)
assert 'jtbl' not in r2
wr(R + '/src/code6cac_b_rodata_pre.c', r2.rstrip('\n') + '\n')
print('stage3 ok')
