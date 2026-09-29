# rv8 layer-2 probes: FAKE-free respellings generated from the landing template (c7)
import re
D = 'tmp/func_800187F4/'
b = open('memory/grind/func_800187F4/template.c', newline='').read()
def w(tag, s):
    assert s != b or tag == 'rv8_base', tag
    open(D + tag + '.c', 'w', newline='\n').write(s)
def rep(s, a, c, n=1):
    assert s.count(a) == n, (a, s.count(a))
    return s.replace(a, c)
w('rv8_base', b)
# ---- delta per-value (dg / dy0) helper
def dl_pv(s):
    s = rep(s, '            s32 delta;\n', '            s32 dg;\n')
    s = rep(s, '            delta = SCR->pos[1] - SCR->ground;\n            if (delta > 0) {\n                if (delta > 0x3200) {',
               '            dg = SCR->pos[1] - SCR->ground;\n            if (dg > 0) {\n                if (dg > 0x3200) {')
    s = rep(s, 'vy_new = vy - delta / 8;', 'vy_new = vy - dg / 8;')
    s = rep(s, '                s32 dx0, dz0, dy1, dx1, dz1;\n', '                s32 dy0, dx0, dz0, dy1, dx1, dz1;\n')
    s = rep(s, '                delta = SCR->cpos[1] - SCR->sph[idx][1];\n                if (delta < -r || r < delta) {',
               '                dy0 = SCR->cpos[1] - SCR->sph[idx][1];\n                if (dy0 < -r || r < dy0) {')
    s = rep(s, 'SCR->d0[1] = delta;', 'SCR->d0[1] = dy0;')
    assert 'delta' not in re.sub(r'/\*.*?\*/', '', s, flags=re.S), 'delta left'
    return s
# P1: delta per-value + named `step` for the /8 (fresh single-value local)
s = dl_pv(b)
s = rep(s, '            s32 dg;\n', '            s32 dg;\n            s32 step;\n')
s = rep(s, '                    vy_new = vy - dg / 8;\n', '                    step = dg / 8;\n                    vy_new = vy - step;\n')
w('rv8_dl_step', s)
# P2: delta per-value, ground depth tested against zero from the other side: `if (0 < dg)` and `if (0x3200 < dg)`
s = dl_pv(b)
s = rep(s, 'if (dg > 0) {', 'if (0 < dg) {'); s = rep(s, 'if (dg > 0x3200) {', 'if (0x3200 < dg) {')
w('rv8_dl_rev', s)
# P3: delta per-value, dg declared unsigned-free s32 at node-loop scope next to idx/nforce
s = dl_pv(b)
s = rep(s, '            s32 dg;\n\n', '\n')
s = rep(s, '        s32 nforce;\n', '        s32 nforce;\n        s32 dg;\n')
w('rv8_dl_nodescope', s)
# ---- work per-value (sq1 / dist1)
def wk_pv(s):
    s = rep(s, '                s32 work;\n', '                s32 sq1, dist1;\n')
    s = rep(s, '                work = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                temp = work;\n                if (work < 0x400) {\n                    work = (&D_8008D118)[work] >> 3;',
               '                sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];\n                temp = sq1;\n                if (sq1 < 0x400) {\n                    dist1 = (&D_8008D118)[sq1] >> 3;')
    s = rep(s, 'temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16)', 'temp = (&D_8008D118)[sq1 >> nbits];\n                    dist1 = (temp << 16)')
    s = rep(s, 'if (work >= r) {', 'if (dist1 >= r) {')
    s = rep(s, 'tot = work + dist2;', 'tot = dist1 + dist2;')
    s = rep(s, 'if (work != 0) {\n                    work = pen / work;', 'if (dist1 != 0) {\n                    dist1 = pen / dist1;')
    s = rep(s, '@gte_lddp(work);', '@gte_lddp(dist1);')
    assert not re.search(r'\bwork\b', re.sub(r'/\*.*?\*/', '', s, flags=re.S)), 'work left'
    return s
# P4: work per-value with the root arms inverted (LZC arm first)
s = wk_pv(b)
m = re.search(r'                if \(sq1 < 0x400\) \{\n(                    dist1 = \(&D_8008D118\)\[sq1\] >> 3;\n)                \} else \{\n(.*?)\n                \}\n                if \(dist1 >= r\)', s, re.S)
s = s[:m.start()] + '                if (sq1 >= 0x400) {\n' + m.group(2) + '\n                } else {\n' + m.group(1) + '                }\n                if (dist1 >= r)' + s[m.end():]
w('rv8_wk_inv', s)
# P5: work per-value, dist1 declared at node-loop scope (outside the ellipsoid loop), sq1 in the ellipsoid body
s = wk_pv(b)
s = rep(s, '                s32 sq1, dist1;\n', '                s32 sq1;\n')
s = rep(s, '        s32 nforce;\n', '        s32 nforce;\n        s32 dist1;\n')
w('rv8_wk_distnode', s)
# ---- nbits: the table shift written as a Ruling 4 compound split, no count local
s = b
s = rep(s, '                    nbits = lz[0];\n                    nbits = 0x16 - (nbits & ~1);\n',
           '                    nbits = 0x16;\n                    nbits -= lz[0] & ~1;\n')
w('rv8_nb_r4split', s)
# P7: nbits per-value, count local of type s32 read into `lzcount`, shift written as `0x16 + -(lzcount & ~1)`? (skip) -> instead: shift from the count in one statement with the count declared first at ellipsoid scope
s = b
s = rep(s, '                    s32 nbits;\n', '                    s32 lzcount, shift;\n')
s = rep(s, '                    nbits = lz[0];\n                    nbits = 0x16 - (nbits & ~1);\n                    temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16) >> (0x13 - (nbits >> 1));',
           '                    lzcount = lz[0];\n                    shift = 0x16 - (lzcount & ~1);\n                    temp = (&D_8008D118)[work >> shift];\n                    work = (temp << 16) >> (0x13 - (shift >> 1));')
s = rep(s, '                    lzcount = lz[0];\n', '                    lzcount = lz[0];\n', 1)
# make it the u32-count + (s32) cast version? keep plain; add ternary-free even computation via subtraction of the low bit
s = rep(s, 'shift = 0x16 - (lzcount & ~1);', 'shift = 0x16 - lzcount + (lzcount & 1);')
w('rv8_nb_lowbit', s)
print('ok')
