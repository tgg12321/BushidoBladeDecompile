V = 'memory/grind/func_800187F4/r11/variants/'
D = 'tmp/func_800187F4/'
def once(s, a, b):
    assert s.count(a) == 1, (a, s.count(a))
    return s.replace(a, b)
dl = open(V + 'r11pv_delta.c').read()
nb = open(V + 'r11pv_nbits.c').read()
nf = open(V + 'r11pv_nforce.c').read()
P = {}
# delta per-value, signed /8 written as the explicit round-toward-zero conditional
P['rv9_dl_rnd'] = once(dl, 'vy_new = vy - dg / 8;', 'vy_new = vy - ((dg < 0 ? dg + 7 : dg) >> 3);')
# delta per-value, the /8 as an if/else on a fresh single-value local
P['rv9_dl_rndif'] = once(once(dl, '            s32 dg;\n', '            s32 dg;\n            s32 q;\n'),
    '                    vy_new = vy - dg / 8;\n',
    '                    if (dg < 0) {\n                        q = (dg + 7) >> 3;\n                    } else {\n                        q = dg >> 3;\n                    }\n                    vy_new = vy - q;\n')
# nbits per-value, & ~1 written as a shift pair
P['rv9_nb_shl'] = once(nb, 'shift = 0x16 - (lzcount & ~1);', 'shift = 0x16 - ((lzcount >> 1) << 1);')
# nforce per-value, each loop's bits loaded before its count
s = once(nf, '        nforce_add = node[7];\n        bits = node[9];\n', '        bits = node[9];\n        nforce_add = node[7];\n')
P['rv9_nf_bitsfirst'] = once(s, '        nforce_sub = node[8];\n        bits2 = node[11];\n', '        bits2 = node[11];\n        nforce_sub = node[8];\n')
# nforce per-value, both counts loaded up front (before the add loop)
s = once(nf, '        nforce_sub = node[8];\n', '')
P['rv9_nf_bothfirst'] = once(s, '        nforce_add = node[7];\n', '        nforce_add = node[7];\n        nforce_sub = node[8];\n')
for k, v in P.items():
    open(D + k + '.c', 'w', newline='\n').write(v)
    open('tmp/rv9/' + k + '.c', 'w', newline='\n').write(v)
print(' '.join(P))
