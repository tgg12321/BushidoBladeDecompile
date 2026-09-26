"""Structural respellings of the per-value twin (v/pv.c) and two-variable partitions of the
reuse body (cand.c).  usage (repo root): python3 tmp/func_80055138/r11/mk2.py"""
import re

D = 'tmp/func_80055138/r11'
cand = open(D + '/cand.c').read()
pv = open(D + '/v/pv.c').read()


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


out = {}
# s_u8: per-value twin with every value's variable typed u8 (the values all fit a byte,
# except the mask word, which stays s32)
s = pv
for n in ('lvl5', 'lvl3', 'row_idx', 'stat1', 'stat2'):
    s = one(s, 's32 %s;' % n, 'u8 %s;' % n)
out['s_u8'] = s
# s_decl_init: per-value twin with each value declared with its initializer
s = pv
for n, rhs in (('lvl5', 'D_800A37D2 / 5'), ('lvl3', 'D_800A37D2 / 3'), ('stat1', 'e[1]'),
               ('stat2', 'e[2]')):
    s = re.sub(r's32 %s;\n(\s*)%s = %s;' % (n, n, re.escape(rhs)),
               r's32 %s = %s;' % (n, rhs.replace('\\', '')), s, count=1)
s = re.sub(r's32 move_mask;\n\s*move_mask = ', 's32 move_mask = ', s, count=1)
out['s_decl_init'] = s
# s_tests: per-value twin, stat tests folded onto the stat variable
s = pv
s = one(s, """                if (e[1] != 0 && e[1] != 0xFF) {
                    s32 stat1;
                    stat1 = e[1];
                    if (stat1 < lo) {
                        lo = stat1;
                    }
                }
""", """                {
                    s32 stat1;
                    stat1 = e[1];
                    if (stat1 != 0 && stat1 != 0xFF && stat1 < lo) {
                        lo = stat1;
                    }
                }
""")
out['s_tests'] = s

# two-variable partitions of the reuse body: temp keeps one group, temp2 the other
CASEV = [('temp = D_800A37D2 / 5;', 'X = D_800A37D2 / 5;'),
         ('= temp * 0x180 + 0x280;', '= X * 0x180 + 0x280;'),
         ('temp = D_800A37D2 / 3;', 'X = D_800A37D2 / 3;'),
         ('if (temp >= 3) {', 'if (X >= 3) {'),
         ('temp = 0;\n', 'X = 0;\n'),
         ('= (temp + 2) << 10;', '= (X + 2) << 10;'),
         ('= 0x3C - temp * 15;', '= 0x3C - X * 15;'),
         ('temp = (u8)(D_800A38E2 / 10) * 2;', 'X = (u8)(D_800A38E2 / 10) * 2;'),
         ('temp--;', 'X--;'),
         ('pair = D_8009A9B4[temp];', 'pair = D_8009A9B4[X];')]
MASKV = [('temp = (e[8] << 24)', 'X = (e[8] << 24)'),
         ('if (!(temp & bit)) {', 'if (!(X & bit)) {')]


def part(repl):
    s = one(cand, '    s32 temp;\n', '    s32 temp;\n    s32 temp2;\n')
    for o, n in repl:
        s = one(s, o, n.replace('X', 'temp2'))
    return s


out['part_case_loop'] = part(CASEV)            # temp2 = the three case values; temp = loop values
out['part_mask_rest'] = part(MASKV)            # temp2 = the mask word; temp = the rest
out['part_casemask_stats'] = part(CASEV + MASKV)  # temp2 = case values + mask; temp = the stats
# hi2_val staging receipts: the shared base in its own local (function / block scope)
i = cand.index('                /* FAKE: the shared base')
j = cand.index('                hi2_val = *(s16 *)(rec + 0x40A) + 100;')
s = cand[:i] + cand[j:]
s = one(s, """                hi2_val = *(s16 *)(rec + 0x40A) + 100;
                lo_val = hi2_val + lo * 40;
                hi1_val = hi2_val + hi1 * 40;
                hi2_val += hi2 * 40;
""", """                base_val = *(s16 *)(rec + 0x40A) + 100;
                lo_val = base_val + lo * 40;
                hi1_val = base_val + hi1 * 40;
                hi2_val = base_val + hi2 * 40;
""")
out['nostage_fs'] = one(s, '    s32 lo_val, hi1_val, hi2_val;\n',
                        '    s32 lo_val, hi1_val, hi2_val;\n    s32 base_val;\n')
out['nostage_blk'] = one(s, '            } else {\n                base_val =',
                         '            } else {\n                s32 base_val;\n                base_val =')
out['nostage_inline'] = one(s, """                base_val = *(s16 *)(rec + 0x40A) + 100;
                lo_val = base_val + lo * 40;
                hi1_val = base_val + hi1 * 40;
                hi2_val = base_val + hi2 * 40;
""", """                lo_val = *(s16 *)(rec + 0x40A) + 100 + lo * 40;
                hi1_val = *(s16 *)(rec + 0x40A) + 100 + hi1 * 40;
                hi2_val = *(s16 *)(rec + 0x40A) + 100 + hi2 * 40;
""")
for k, v in out.items():
    open('%s/v/%s.c' % (D, k), 'w', newline='\n').write(v)
print(' '.join(sorted(out)))
