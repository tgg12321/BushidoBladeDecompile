"""7C-base respelling variants (i7c_*)."""
import os, re, shutil
B = 'tmp/func_80034708/i7c'
def mk(name, hfix=None, sfix=None):
    O = f'tmp/func_80034708/i7c_{name}'
    shutil.rmtree(O, ignore_errors=True)
    shutil.copytree(B + '/include', O + '/include')
    os.makedirs(O + '/src')
    h = open(B + '/include/code6cac.h').read()
    s = open(B + '/src/code6cac_b3.c').read()
    if hfix: h = hfix(h)
    if sfix: s = sfix(s)
    open(O + '/include/code6cac.h', 'w', newline='\n').write(h)
    open(O + '/src/code6cac_b3.c', 'w', newline='\n').write(s)

# s8 fields, casts dropped
def h_s8(h):
    i = h.index('typedef struct {\n    u8 unk_0[2];')
    j = h.index('} PlayerBytePairs;')
    return h[:i] + h[i:j].replace('u8 ', 's8 ') + h[j:]
mk('s8', h_s8, lambda s: s.replace('(s8)D_8010277C', 'D_8010277C'))
# 2-D pairs
def h_2d(h):
    return h.replace('''    u8 unk_0[2];
    u8 unk_2[2];
    u8 unk_4[2];
    u8 unk_6[2];
    u8 unk_8;''', '''    u8 unk_0[4][2];
    u8 unk_8;''')
def s_2d(s):
    s = re.sub(r'unk_0\[', 'unk_0[0][', s)
    s = re.sub(r'unk_2\[', 'unk_0[1][', s)
    s = re.sub(r'unk_4\[', 'unk_0[2][', s)
    return s
mk('2d', h_2d, s_2d)
# row reads through one pointer to the block (rows 1-6)
def s_ptr(s):
    s = s.replace('    u8 *on;\n', '    u8 *on;\n    u8 *prm;\n', 1)
    s = s.replace('    on = D_800A3180;\n', '    on = D_800A3180;\n    prm = D_8010277C.unk_0;\n', 1)
    for k, off in (('unk_0[0]', 0), ('unk_0[1]', 1), ('unk_2[0]', 2), ('unk_2[1]', 3), ('unk_4[0]', 4), ('unk_4[1]', 5)):
        s = s.replace(f'(s8)D_8010277C.{k});', f'(s8)prm[{off}]);', 1)
    return s
mk('ptr', None, s_ptr)
print('ok')
