"""Operand-order probes for the strip16 add on the h2 chassis (BA00 alias at loop top + E1)."""
D = 'tmp/func_800620B8/s4/'
out = {}
for ch in ('h2_ba00_top', 'h4_ba00_ba30_top', 'h1_all_top'):
    b = open(D + ch + '.c', encoding='utf-8').read()
    S = '(s32)strip16[D_800A32B8 & 3]'
    assert b.count(S) == 1
    out[ch + '_r1'] = b.replace(S, '(s32)strip16[D_800A32B8 & 3] - (s32)strip16 + (s32)strip16 /* FAKE */')
    out[ch + '_r2'] = b.replace(S, '(D_800A32B8 & 3) * 8 + (s32)strip16')
    out[ch + '_r3'] = b.replace(S, '(s32)strip16 + ((s32)strip16[D_800A32B8 & 3] - (s32)strip16) /* FAKE */')
    out[ch + '_r4'] = b.replace(S, '(s32)strip16[D_800A32B8 & 3] + (s32)strip16 - (s32)strip16 /* FAKE */')
for k, v in out.items():
    open(D + k + '.c', 'w', encoding='utf-8', newline='\n').write(v)
open(D + 'list3.txt', 'w').write(','.join(D + k + '.c' for k in out))
