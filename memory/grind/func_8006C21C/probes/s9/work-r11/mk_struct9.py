#!/usr/bin/env python3
"""Ruling 11 (D)(4) structural respellings of the one-variable-per-value body w_spl_all.c."""
import re
S = open('tmp/c21c9/w_spl_all.c', newline='').read()
MEM = '*(s16 *)(D_800A34FC + j * 2 + 0x28)'
V = {}
# st_memtest: bar tests re-read the level from memory; `level` only indexes rec
t = S.replace('if (level == 5)', f'if ({MEM} == 5)')
assert t.count(f'if ({MEM} == 5)') == 2
V['st_memtest'] = t
# st_while: the three counter loops as while loops
t = S
for v, n in (('bit', 4), ('sprite', 6), ('trow', 11)):
    pat = re.compile(r'( *)for \(%s = 0; %s < %d; %s\+\+\) \{\n' % (v, v, n, v))
    m = pat.search(t)
    assert m, v
    ind = m.group(1)
    # find the matching close brace of this loop body
    i = m.end(); depth = 1
    while depth:
        c = t[i]
        depth += c == '{'
        depth -= c == '}'
        i += 1
    close = i - 1
    body = t[m.end():close]
    t = (t[:m.start()] + f'{ind}{v} = 0;\n{ind}while ({v} < {n}) {{\n' + body
         + f'    {v}++;\n{ind}' + '}' + t[close + 1:])
V['st_while'] = t
# st_s16level: the level is the s16 field's own type
t = S.replace('        s32 level = ', '        s16 level = ')
assert t != S
V['st_s16level'] = t
# st_levelptr: the bar tests compare the record pointer instead of the level
t = S.replace('if (level == 5)', 'if (rec == &recs[6])')
V['st_recptr'] = t
for k, s in V.items():
    open(f'tmp/c21c9/{k}.c', 'w', newline='').write(s)
print(' '.join(f'tmp/c21c9/{k}.c' for k in V))
