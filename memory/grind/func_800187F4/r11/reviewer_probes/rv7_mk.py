# rv7 (seventh layer-2) probe generator: FAKE-free one-variable-per-value spellings of c7.
import re
D = 'tmp/func_800187F4/'
P = 'memory/grind/func_800187F4/r11/reviewer_probes/'
c7 = open(D + 'c7.c').read()
wk = open(P + 'rv5_wk_pv.c').read()      # work per-value (sq1/dist1), banked 26
dl = open(P + 'rv5_dl_coll_dgfirst.c').read()  # delta per-value (dg/dy0), banked 4

def rep(s, a, b, n=1):
    assert s.count(a) >= 1, a
    return s.replace(a, b, n if n else -1)

out = {}
# 1. work per-value, sq1 accumulated by split-init (ordinary C per split-init ruling)
t = rep(wk, 'sq1 = SCR->sq[0] + SCR->sq[1] + SCR->sq[2];',
        'sq1 = SCR->sq[0];\n                sq1 += SCR->sq[1];\n                sq1 += SCR->sq[2];')
out['rv7_wk_splitinit'] = t
# 2. delta per-value, dg by split-init
t = rep(dl, 'dg = SCR->pos[1] - SCR->ground;', 'dg = SCR->pos[1];\n            dg -= SCR->ground;')
out['rv7_dl_splitinit'] = t
# 3. nforce per-value, counts loaded in the for-init comma expression
t = c7
t = rep(t, '        s32 nforce;\n', '        s32 nf_add;\n        s32 nf_sub;\n')
t = rep(t, '        nforce = node[7];\n        bits = node[9];\n        for (idx = 0; idx < nforce; idx++) {',
        '        bits = node[9];\n        for (idx = 0, nf_add = node[7]; idx < nf_add; idx++) {')
t = rep(t, '        nforce = node[8];\n        bits2 = node[11];\n        for (idx = 0; idx < nforce; idx++) {',
        '        bits2 = node[11];\n        for (idx = 0, nf_sub = node[8]; idx < nf_sub; idx++) {')
out['rv7_nf_forinit'] = t
# 4. inline helper for the LUT root (per-value sq / root / copy / byte / count / shift)
helper = '''static inline s32 lut_root(s32 sq, s32 *lzp) {
    s32 root;

    if (sq < 0x400) {
        root = (&D_8008D118)[sq] >> 3;
    } else {
        s32 lzcount;
        s32 shift;
        s32 byte;

        @gte_Lzc(sq, lzp);
        lzcount = *lzp;
        shift = 0x16 - (lzcount & ~1);
        byte = (&D_8008D118)[sq >> shift];
        root = (byte << 16) >> (0x13 - (shift >> 1));
    }
    return root;
}
'''
t = wk
i = t.index('void func_800187F4(')
t = t[:i] + helper + t[i:]
a = t.index('                temp = sq1;\n                if (sq1 < 0x400) {')
b = t.index('                if (dist1 >= r) {')
t = t[:a] + '                dist1 = lut_root(sq1, &lz[0]);\n' + t[b:]
a = t.index('                if (sq2 < 0x400) {')
b = t.index('                tot = dist1 + dist2;')
t = t[:a] + '                dist2 = lut_root(sq2, &lz[1]);\n' + t[b:]
t = rep(t, '''                /* Ruling 11 (proof r11/proof.md): three values -- a copy of the
                 * squared length for the leading-zero-count macro (a value under
                 * (C)(3)'s GTE-macro input copy clause, owner ruling 2026-09-28
                 * Q28), then the focus-0 table byte, then the focus-1 table byte. */
                s32 temp;
''', '')
out['rv7_inl_root'] = t
# 5. the same helper with the copy kept at the first call site (Q28 copy as its own local)
t2 = rep(t, '                dist1 = lut_root(sq1, &lz[0]);\n',
         '                {\n                    s32 lzc_in = sq1;\n\n                    dist1 = lut_root(lzc_in, &lz[0]);\n                }\n')
out['rv7_inl_root_copy'] = t2
# 6. temp per-value: table bytes read straight into the shift expression (no byte local), copy as own local
t = c7
t = rep(t, '                temp = work;\n', '                lzc_in = work;\n')
t = rep(t, '@gte_Lzc(temp, &lz[0]);', '@gte_Lzc(lzc_in, &lz[0]);')
t = rep(t, '                    temp = (&D_8008D118)[work >> nbits];\n                    work = (temp << 16) >> (0x13 - (nbits >> 1));',
        '                    work = ((&D_8008D118)[work >> nbits] << 16) >> (0x13 - (nbits >> 1));')
t = rep(t, '                    temp = (&D_8008D118)[sq2 >> nbits2];\n                    dist2 = (temp << 16) >> (0x13 - (nbits2 >> 1));',
        '                    dist2 = ((&D_8008D118)[sq2 >> nbits2] << 16) >> (0x13 - (nbits2 >> 1));')
t = rep(t, '                s32 temp;\n', '                s32 lzc_in;\n')
out['rv7_tp_nobyte_copy'] = t
# 7. frame probe lz[8] on c7
out['rv7_lz8'] = rep(c7, '    s32 lz[6];', '    s32 lz[8];')
out['rv7_lz2'] = rep(c7, '    s32 lz[6];', '    s32 lz[2];')
out['rv7_lz3'] = rep(c7, '    s32 lz[6];', '    s32 lz[3];')
# 8. all per-value with split-init sums (work + delta), idx/nforce/nbits still shared
t = rep(out['rv7_wk_splitinit'], '''            s32 delta;

            delta = SCR->pos[1] - SCR->ground;
            if (delta > 0) {
                if (delta > 0x3200) {''', '''            s32 dg;
            s32 dy0;

            dg = SCR->pos[1];
            dg -= SCR->ground;
            if (dg > 0) {
                if (dg > 0x3200) {''')
t = rep(t, 'vy_new = vy - delta / 8;', 'vy_new = vy - dg / 8;')
t = rep(t, '''                delta = SCR->cpos[1] - SCR->sph[idx][1];
                if (delta < -r || r < delta) {''', '''                dy0 = SCR->cpos[1] - SCR->sph[idx][1];
                if (dy0 < -r || r < dy0) {''')
t = rep(t, 'SCR->d0[1] = delta;', 'SCR->d0[1] = dy0;')
out['rv7_wkdl_splitinit'] = t
for k, v in out.items():
    open(D + k + '.c', 'w', newline='\n').write(v)
print(' '.join(out))
