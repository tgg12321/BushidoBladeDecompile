"""Duplicated-statement-into-arms (F7 / duplicated-statement-into-arms family) variants of the
per-value twins: the statements that follow a conditional and read the split value are
duplicated into both arms of that conditional (jump2 cross-jump can re-merge them).
usage (repo root): python3 tmp/func_80055138/r11/mk4.py -> v/dup_*.c"""
D = 'tmp/func_80055138/r11'
pv = open(D + '/v/pv.c').read()
ctr = open(D + '/v/ctr_split.c').read()
F = ' /* FAKE: duplicated into both arms */'


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


def ind(block, k):
    return ''.join((' ' * k + l if l.strip() else l) for l in block.splitlines(True))


out = {}
# value 2 (lvl3): the five statements after `if (lvl3 >= 3) {...}` into both arms
tail3 = """            p[0x443] = 0x19;
            *(s16 *)(p + 0x1C) = (lvl3 + 2) << 10;
            *(s16 *)(p + 0x438) = 0;
            p[0x424] = 0;
            p[0x3F6] = 0x3C - lvl3 * 15;
"""
old = """            if (lvl3 >= 3) {
                D_800A37D2 = 0;
                lvl3 = 0;
            }
""" + tail3
out['dup_lvl3'] = one(pv, old, """            if (lvl3 >= 3) {""" + F + """
                D_800A37D2 = 0;
                lvl3 = 0;
""" + ind(tail3, 4) + """            } else {
""" + ind(tail3, 4) + """            }
""")
# value 3 (row_idx): the three statements after `if (... % 10 == 0) row_idx--;`
tailr = """        pair = D_8009A9B4[row_idx];
        p[0x424] = pair[0];
        p[0x3F6] = pair[1];
"""
old = """        if ((u8)(D_800A38E2 % 10) == 0) {
            row_idx--;
        }
""" + tailr
out['dup_row_idx'] = one(pv, old, """        if ((u8)(D_800A38E2 % 10) == 0) {""" + F + """
            row_idx--;
""" + ind(tailr, 4) + """        } else {
""" + ind(tailr, 4) + """        }
""")
# value 6 (stat2): `cat = ...; if (hi2 < stat2 ...) hi2 = stat2;` into both arms of `if (hi1 < stat2)`
tails = """                    cat = e[0] & 7;
                    if (hi2 < stat2 && (e[3] & 0xF) * 4 < 0x10 && (cat < 2 || cat == 7)) {
                        hi2 = stat2;
                    }
"""
old = """                    if (hi1 < stat2) {
                        hi1 = stat2;
                    }
""" + tails
out['dup_stat2'] = one(pv, old, """                    if (hi1 < stat2) {""" + F + """
                        hi1 = stat2;
""" + ind(tails, 4) + """                    } else {
""" + ind(tails, 4) + """                    }
""")
# value 5 (stat1) / value 4 (mask) / value 1 (lvl5): the value's statements into both arms of the
# conditional that precedes them in the same block chain
old = """                if (e[1] != 0 && e[1] != 0xFF) {
                    s32 stat1;
                    stat1 = e[1];
                    if (stat1 < lo) {
                        lo = stat1;
                    }
                }
"""
blk1 = old
out['dup_stat1'] = one(pv, """                    if (!(move_mask & bit)) {
                        goto next;
                    }
                }
""" + old, """                    if (!(move_mask & bit)) {
                        goto next;
                    }
""" + ind(blk1, 4) + """                } else {""" + F + """
""" + ind(blk1, 4) + """                }
""")
blk5 = """            s32 lvl5;
            lvl5 = D_800A37D2 / 5;
            *(s16 *)(p + 0x438) = lvl5 * 0x180 + 0x280;
"""
# value 1: the level statements duplicated into both arms of the (already present) mode
# test that precedes the arm, i.e. hoisted as the first statements of if/else of D_800A389A
# is impossible without moving them; instead duplicate the clamp-and-call tail into the
# clamp's arms so the store's value is read in two blocks
tail5 = """            if ((u8)(D_800A37D2 % 5) == 0) {
                func_8005509C(*(s16 *)(p + 4));
            }
"""
old = """            if (*(s16 *)(p + 0x438) > 0x1000) {
                *(s16 *)(p + 0x438) = 0x1000;
            }
""" + tail5
out['dup_lvl5_tail'] = one(pv, old, """            if (*(s16 *)(p + 0x438) > 0x1000) {""" + F + """
                *(s16 *)(p + 0x438) = 0x1000;
""" + ind(tail5, 4) + """            } else {
""" + ind(tail5, 4) + """            }
""")
# the clear loop's counter: the clear loop duplicated into both arms of the rand() test
loopc = """    for (zero_i = 0; zero_i < 8U; zero_i++) {
        (p + zero_i)[0x444] = 0;
    }
"""
old = """    if (D_80099D88[p[0x443]].flags & 0x100) {
        D_80099D88[p[0x443]].unk3 = (rand() & 3) + 1;
    }
""" + loopc
out['dup_zero_i'] = one(ctr, old, """    if (D_80099D88[p[0x443]].flags & 0x100) {""" + F + """
        D_80099D88[p[0x443]].unk3 = (rand() & 3) + 1;
""" + ind(loopc, 4) + """    } else {
""" + ind(loopc, 4) + """    }
""")
for k, v in out.items():
    open('%s/v/%s.c' % (D, k), 'w', newline='\n').write(v)
print(' '.join(sorted(out)))
