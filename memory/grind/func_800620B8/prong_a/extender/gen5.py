"""Honest operand-order spellings for the sel_b add on m_no_x16 (strip32 extender kept, strip16 extender dropped)."""
D = 'tmp/func_800620B8/s4/'
b = open(D + 'm_no_x16.c', encoding='utf-8').read()
S = '(s32)strip16[D_800A32B8 & 3]'
assert b.count(S) == 1
V = {
    'p2': '(s32)strip16 + (D_800A32B8 & 3) * sizeof(*strip16)',
    'p3': '(D_800A32B8 & 3) * sizeof(*strip16) + (s32)strip16',
    'p4': '(s32)&strip16[D_800A32B8 & 3][0]',
    'p5': '(s32)((u8 *)strip16 + (D_800A32B8 & 3) * sizeof(*strip16))',
    'p6': '(s32)(D_800A32B8 & 3) * 8 + (s32)strip16',
    'p7': '((D_800A32B8 & 3) << 3) + (s32)strip16',
}
for k, v in V.items():
    open(D + k + '.c', 'w', encoding='utf-8', newline='\n').write(b.replace(S, v))
open(D + 'list5.txt', 'w').write(','.join(D + k + '.c' for k in V))
