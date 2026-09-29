#!/usr/bin/env python3
"""What each declaration correction is worth, on the landing body. Writes bodies + TU patch modules."""
import runpy
L = runpy.run_path('tmp/func_8005E54C/land.py')
body = L['body']()
V = 'tmp/func_8005E54C/v/'


def unflags(b):
    b = b.replace('D_8009BD38.unk15 >> i & 1', '(s32)(((u32)D_8009BD38 >> 15) & 3) >> i & 1')
    b = b.replace('D_8009BD38.unk10 + 3', '(s32)(((u32)D_8009BD38 >> 10) & 3) + 3')
    b = b.replace('D_8009BD38.unk10 == 2', '(D_8009BD38 & 0xC00) == 0x800')
    b = b.replace('D_8009BD38.unk10 == 1', '(D_8009BD38 & 0xC00) == 0x400')
    assert 'D_8009BD38.' not in b
    return b


def unrec(b):
    b = b.replace('D_8009BD24[j][i].chr', 'D_8009BD24[j * 10 + i * 2]')
    assert 'D_8009BD24[j]' not in b
    return 'extern u8 D_8009BD24[];\n' + b


GAME = L['GAME_H_ADD']
i38 = GAME.index('/* 0x8009BD38')
i24 = GAME.index('/* 0x8009BD24')
g490 = GAME[:i24]
g24 = GAME[i24:i38]
g38 = GAME[i38:]
open(V + 'w_both.c', 'w', newline='\n').write(body)
open(V + 'w_24only.c', 'w', newline='\n').write(unflags(body))
open(V + 'w_38only.c', 'w', newline='\n').write(unrec(body))
open(V + 'w_none.c', 'w', newline='\n').write(unrec(unflags(body)))

TPL = '''import runpy
L = runpy.run_path('tmp/func_8005E54C/land.py')
ADD = %r
USE24 = %r
USE38 = %r


def patch(s):
    if USE38:
        assert s.count('extern s32 D_8009BD38;\\n') == 1
        s = s.replace('extern s32 D_8009BD38;\\n', '')
        s = s.replace('(((u32)D_8009BD38 >> 14) & 1) + 1', 'D_8009BD38.unk14 + 1')
        s = s.replace('(D_8009BD38 & 0x3000) == 0x2000', 'D_8009BD38.unk12 == 2')
        s = s.replace('v = D_8009BD38 & 0xF;', 'v = D_8009BD38.unk0;')
        s = s.replace('(D_8009BD38 & 0xF) * 2', 'D_8009BD38.unk0 * 2')
        old = """        s32 *p = &D_8009BD38;
        s32 cur;
        ret = 1;
        cur = *p;
        D_800A35E4 = 0;
        cur &= ~0xF;
        cur |= result & 0xF;
        *p = cur;"""
        assert old in s
        s = s.replace(old, """        ret = 1;
        D_800A35E4 = 0;
        D_8009BD38.unk0 = result;""")
    if USE24:
        s = s.replace('extern u8 D_8009BD24[];\\n', '')
        s = s.replace('D_8009BD24[0] < 0xC', 'D_8009BD24[0][0].chr < 0xC')
        s = s.replace('func_8006E534(a0, D_800A35E0, D_8009BD24, D_800A35E8)',
                      'func_8006E534(a0, D_800A35E0, (u8 *)D_8009BD24, D_800A35E8)')
    inc = '#include "code6cac.h"\\n'
    return s.replace(inc, inc + ADD, 1)
'''
for name, u24, u38 in (('both', 1, 1), ('24only', 1, 0), ('38only', 0, 1), ('none', 0, 0)):
    add = g490 + (g24 if u24 else '') + (g38 if u38 else '')
    open('tmp/func_8005E54C/tp_%s.py' % name, 'w', newline='\n').write(TPL % (add, bool(u24), bool(u38)))
print('ok')
