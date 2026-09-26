"""Sanctioned-family escapes on the one-variable-per-value twins: FAKE self-assignments,
dead stores, combine-foldable chain-extenders and a pointer alias placed on the split values
whose local-alloc tie is the necessity argument (lvl5, move_mask) and on the split clear-loop
counter (zero_i).  Each is v/pv.c (or v/ctr_split.c) plus the named construct only.
usage (repo root): python3 tmp/func_80055138/r11/mk3.py  -> v/ce_*.c"""
D = 'tmp/func_80055138/r11'
pv = open(D + '/v/pv.c').read()
ctr = open(D + '/v/ctr_split.c').read()


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


F = ' /* FAKE */'
STORE5 = '            *(s16 *)(p + 0x438) = lvl5 * 0x180 + 0x280;\n'
CLAMP = '                *(s16 *)(p + 0x438) = 0x1000;\n'
CALLIF = '            if ((u8)(D_800A37D2 % 5) == 0) {\n                func_8005509C(*(s16 *)(p + 4));\n            }\n'
DECL5 = '            s32 lvl5;\n'
out = {}
# --- value 1 (lvl5) ---
out['ce_l_self_same'] = one(pv, STORE5, STORE5 + '            lvl5 = lvl5;' + F + '\n')
out['ce_l_self_clamp'] = one(pv, CLAMP, CLAMP + '                lvl5 = lvl5;' + F + '\n')
out['ce_l_self_after'] = one(pv, CALLIF, CALLIF + '            lvl5 = lvl5;' + F + '\n')
out['ce_l_dead_clamp'] = one(pv, CLAMP, CLAMP + '                lvl5 = 0;' + F + '\n')
s = one(pv, DECL5, '')
s = one(s, '    u8 base;\n', '    u8 base;\n    s32 lvl5;\n')
out['ce_l_dead_init'] = one(s, '    p[0x443] = *(u16 *)(p + 0xA);\n',
                            '    lvl5 = 0;' + F + '\n    p[0x443] = *(u16 *)(p + 0xA);\n')
out['ce_l_chain_clamp'] = one(pv, CLAMP, '                *(s16 *)(p + 0x438) = 0x1000 + lvl5 - lvl5;' + F + '\n')
out['ce_l_chain_callif'] = one(pv, '            if ((u8)(D_800A37D2 % 5) == 0) {\n',
                               '            if ((u8)(D_800A37D2 % 5) + lvl5 - lvl5 == 0) {' + F + '\n')
out['ce_l_chain_callarg'] = one(pv, '                func_8005509C(*(s16 *)(p + 4));\n            }\n        } else {',
                                '                func_8005509C(*(s16 *)(p + 4) + lvl5 - lvl5);' + F + '\n            }\n        } else {')
out['ce_l_chain_same'] = one(pv, STORE5, '            *(s16 *)(p + 0x438) = (lvl5 + 1) * 0x180 + 0x280 - 0x180;' + F + '\n')
s = one(pv, DECL5, DECL5 + '            s32 *lvl5_p;\n')
out['ce_l_alias'] = one(s, STORE5, '            lvl5_p = &lvl5;' + F + '\n            *(s16 *)(p + 0x438) = *lvl5_p * 0x180 + 0x280;\n')
# --- value 4 (move_mask) ---
TEST = '                    if (!(move_mask & bit)) {\n                        goto next;\n                    }\n'
out['ce_m_self_after'] = one(pv, TEST, TEST + '                    move_mask = move_mask;' + F + '\n')
out['ce_m_dead_goto'] = one(pv, TEST, '                    if (!(move_mask & bit)) {\n                        move_mask = 0;' + F + '\n                        goto next;\n                    }\n')
out['ce_m_chain_goto'] = one(pv, TEST, '                    if (!(move_mask & bit)) {\n                        cursor += move_mask - move_mask;' + F + '\n                        goto next;\n                    }\n')
s = one(pv, '                    s32 move_mask;\n', '')
s = one(s, '                u8 *e = list + *cursor;\n', '                u8 *e = list + *cursor;\n                s32 move_mask;\n')
out['ce_m_chain_e1'] = one(s, '                if (e[1] != 0 && e[1] != 0xFF) {\n',
                           '                if (e[1] + move_mask - move_mask != 0 && e[1] != 0xFF) {' + F + '\n')
# --- the clear loop's counter (zero_i) ---
out['ce_z_self_after'] = one(ctr, '    *(u16 **)(p + 0x3A4) = arg1;\n',
                             '    zero_i = zero_i;' + F + '\n    *(u16 **)(p + 0x3A4) = arg1;\n')
out['ce_z_chain_ploop'] = one(ctr, '    for (idx = 0; idx < 2; idx++) {\n',
                              '    for (idx = 0; idx < 2 + zero_i - zero_i; idx++) {' + F + '\n')
out['ce_z_chain_scan'] = one(ctr, '                cursor++;\n            }\n',
                             '                cursor += 1 + zero_i - zero_i;' + F + '\n            }\n')
out['ce_z_dead_end'] = one(ctr, '    other = *(u8 **)p;\n', '    zero_i = 0;' + F + '\n    other = *(u8 **)p;\n')
for k, v in out.items():
    open('%s/v/%s.c' % (D, k), 'w', newline='\n').write(v)
print(' '.join(sorted(out)))
