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
