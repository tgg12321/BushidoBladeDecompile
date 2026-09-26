"""Generate func_80055138 Ruling 11 variants from ONE base, tmp/func_80055138/r11/cand.c
(the body to be committed).  Every variant differs from cand.c ONLY by the named carrier
split: added declarations + renamed identifiers (and, for the full twin, temp's own
declaration removed).  usage (repo root): python3 tmp/func_80055138/r11/mk.py
Writes tmp/func_80055138/r11/v/<name>.c ."""
import os

D = 'tmp/func_80055138/r11'
os.makedirs(D + '/v', exist_ok=True)
base = open(D + '/cand.c').read()


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


# The six values of `temp`.  'rn' = (old, new) statement-text renames (the write and
# every read of that value); 'scope' = the opening line of the innermost block that
# encloses the value's writes (its own declaration goes right after it).
VAL = {
    'lvl5': dict(scope='        if (D_800A389A) {\n', rn=[
        ('temp = D_800A37D2 / 5;', 'lvl5 = D_800A37D2 / 5;'),
        ('= temp * 0x180 + 0x280;', '= lvl5 * 0x180 + 0x280;')]),
    'lvl3': dict(scope='        } else {\n            temp = D_800A37D2 / 3;', rn=[
        ('temp = D_800A37D2 / 3;', 'lvl3 = D_800A37D2 / 3;'),
        ('if (temp >= 3) {', 'if (lvl3 >= 3) {'),
        ('temp = 0;\n', 'lvl3 = 0;\n'),
        ('= (temp + 2) << 10;', '= (lvl3 + 2) << 10;'),
        ('= 0x3C - temp * 15;', '= 0x3C - lvl3 * 15;')]),
    'row_idx': dict(scope='    switch (D_800A38DC) {\n', rn=[
        ('temp = (u8)(D_800A38E2 / 10) * 2;', 'row_idx = (u8)(D_800A38E2 / 10) * 2;'),
        ('temp--;', 'row_idx--;'),
        ('pair = D_8009A9B4[temp];', 'pair = D_8009A9B4[row_idx];')]),
    'move_mask': dict(scope='                if (e[4] == 0x40) {\n', rn=[
        ('temp = (e[8] << 24)', 'move_mask = (e[8] << 24)'),
        ('if (!(temp & bit)) {', 'if (!(move_mask & bit)) {')]),
    'stat1': dict(scope='                if (e[1] != 0 && e[1] != 0xFF) {\n', rn=[
        ('temp = e[1];', 'stat1 = e[1];'),
        ('if (temp < lo) {', 'if (stat1 < lo) {'),
        ('lo = temp;', 'lo = stat1;')]),
    'stat2': dict(scope='                if (e[2] != 0 && e[2] != 0xFF) {\n', rn=[
        ('temp = e[2];', 'stat2 = e[2];'),
        ('if (hi1 < temp) {', 'if (hi1 < stat2) {'),
        ('hi1 = temp;', 'hi1 = stat2;'),
        ('if (hi2 < temp &&', 'if (hi2 < stat2 &&'),
        ('hi2 = temp;', 'hi2 = stat2;')]),
}
ALL = ['lvl5', 'lvl3', 'row_idx', 'move_mask', 'stat1', 'stat2']
TEMP_DECL_END = '    s32 temp;\n'


def indent_of(scope_line):
    last = scope_line.rstrip('\n').split('\n')[0]
    return ' ' * (len(last) - len(last.lstrip()) + 4)


def split(s, names, where='inner'):
    for n in names:
        if where == 'inner':
            sc = VAL[n]['scope']
            head = sc.split('\n')[0] + '\n'
            s = one(s, sc, head + indent_of(sc) + 's32 %s;\n' % n + sc[len(head):])
        else:  # function scope, right after temp's declaration
            s = one(s, TEMP_DECL_END, TEMP_DECL_END + '    s32 %s;\n' % n)
    for n in names:
        for o, nw in VAL[n]['rn']:
            s = one(s, o, nw)
    return s


def drop_temp(s):
    # remove temp's declaration and its comment block (the twin has no temp)
    i = s.index('    /* temp holds six values')
    j = s.index(TEMP_DECL_END) + len(TEMP_DECL_END)
    return s[:i] + s[j:]


out = {'cand': base}
out['pv'] = drop_temp(split(base, ALL))                   # full one-variable-per-value twin
out['pv_fs'] = drop_temp(split(base, ALL, 'fs'))          # structural: all at function scope
for n in ALL:                                             # ablation: one value split, rest shared
    out['abl_' + n] = split(base, [n])
# the counter: zero loop gets its own counter
cs = one(base, '    for (idx = 0; idx < 8U; idx++) {\n        (p + idx)[0x444] = 0;\n',
         '    for (zero_i = 0; zero_i < 8U; zero_i++) {\n        (p + zero_i)[0x444] = 0;\n')
cs = one(cs, '    s32 idx;\n', '    s32 idx;\n    s32 zero_i;\n')
out['ctr_split'] = cs
for k, v in out.items():
    open('%s/v/%s.c' % (D, k), 'w', newline='\n').write(v)
print(' '.join(sorted(out)))
