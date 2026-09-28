"""cand10 = banked candidate with mode set once at the top and used at every func_8006E480 call;
plus the one-variable-per-value seeds for the i (level) and cells admission records."""
NL = chr(10)
s = open('memory/grind/func_8006C21C/candidate.c').read()
a = s.replace('func_8006E480((s32)s.header, 0), 0);', 'func_8006E480((s32)s.header, mode), 0);', 1)
a = a.replace('    mode = 0;' + NL, '', 1).replace('    s.ot_idx = 10;' + NL, '    mode = 0;' + NL + '    s.ot_idx = 10;' + NL, 1)
assert a.count('func_8006E480((s32)s.header, mode)') == 3
open('tmp/c21c/cand10.c', 'w', newline=NL).write(a)

# separate level (s32, function scope)
b = a.replace('        i = *(s16 *)(D_800A34FC + j * 2 + 0x28);' + NL + '        rec = &recs[i + 1];',
              '        level = *(s16 *)(D_800A34FC + j * 2 + 0x28);' + NL + '        rec = &recs[level + 1];')
b = b.replace('            if (i == 5) {', '            if (level == 5) {')
b = b.replace('    s32 i;' + NL, '    s32 i;' + NL + '    s32 level;' + NL, 1)
assert 'level == 5' in b and 'recs[level + 1]' in b
open('tmp/c21c/lvl10.c', 'w', newline=NL).write(b)

# no cells carrier: each site stores the expression directly
c = a.replace('        cells = s.header + 0xC;' + NL + '        s.table = cells;', '        s.table = s.header + 0xC;')
c = c.replace('    cells = s.header + 0xC;' + NL + '    s.table = cells;', '    s.table = s.header + 0xC;')
c = c.replace('                    cells = s.header + 0xC;' + NL + '                    s.table = cells;', '                    s.table = s.header + 0xC;')
assert 'cells =' not in c, [l for l in c.split(NL) if 'cells =' in l]
c = c.replace('    u8 *cells;' + NL, '', 1)
open('tmp/c21c/nocells10.c', 'w', newline=NL).write(c)

# mode as literal at every site
d = a.replace('func_8006E480((s32)s.header, mode)', 'func_8006E480((s32)s.header, 0)')
d = d.replace('    mode = 0;' + NL, '', 1).replace('    s32 mode;' + NL, '', 1)
open('tmp/c21c/modelit10.c', 'w', newline=NL).write(d)
print('ok')
