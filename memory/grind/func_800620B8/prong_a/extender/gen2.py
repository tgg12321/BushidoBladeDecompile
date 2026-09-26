"""Hybrids: f1 chassis + E1 extender, with some/all table pointers set at the loop top (loop.c hoists)."""
D = 'tmp/func_800620B8/s4/'
base = open(D + 'f1_e1.c', encoding='utf-8').read()
SETS = ['    strip32 = D_8009BA00;\n', '    alt32 = D_8009BA50;\n', '    strip16 = D_8009BA30;\n', '    alt16 = D_8009BA58;\n']
TOP = '    for (i = 0; D_800F1198[i].unk0 & 1; i++) {\n'


def sub(s, a, b):
    assert s.count(a) == 1, a
    return s.replace(a, b)


def move(src, which):
    s = src
    moved = ''
    for k in which:
        s = sub(s, SETS[k], '')
        moved += '    ' + SETS[k]
    return sub(s, TOP, TOP + moved)


out = {
    'h1_all_top': move(base, [0, 1, 2, 3]),
    'h2_ba00_top': move(base, [0]),
    'h4_ba00_ba30_top': move(base, [0, 2]),
}
S16 = '(s32)strip16[D_800A32B8 & 3]'
for k in list(out):
    out[k + '_idx'] = sub(out[k], S16, '(s32)(strip16 + (D_800A32B8 & 3))')
    out[k + '_rev'] = sub(out[k], S16, '(s32)((D_800A32B8 & 3) + strip16)')
out['f1_e1_rev'] = sub(base, S16, '(s32)((D_800A32B8 & 3) + strip16)')
for k, v in out.items():
    open(D + k + '.c', 'w', encoding='utf-8', newline='\n').write(v)
print(','.join(D + k + '.c' for k in out))
