#!/usr/bin/env python3
"""Ruling 11 (C)/(D)(4) spellings of `work` on the Q27 landing body (tmp/c21c9/land_sb.c).
Values: p2 = phase-2 unlock-bit index, p4 = phase-4 sprite index, p6 = phase-6 tile row,
lv = phase-8 gauge level. One-variable-per-value names: bit, sprite, trow, level; each is
declared at the innermost scope enclosing its writes (level: the j-loop body)."""
import re
S = open('tmp/c21c9/land_sb.c', newline='').read()
L0 = S.split('\n')
def find(pat):
    return [i for i, l in enumerate(L0) if re.search(pat, l)]
GROUPS = {
    'p2': find(r'for \(work = 0; work < 4;') + find(r'\(1 << work\)') + find(r'table\[work \+ 13\]'),
    'p4': find(r'for \(work = 0; work < 6;') + find(r'table\[work \+ 2\]'),
    'p6': find(r'for \(work = 0; work < 11;') + find(r'&recs\[work\];'),
    'lv': find(r'work = \*\(s16 \*\)') + find(r'&recs\[work \+ 1\]') + find(r'if \(work == 5\)'),
}
NAME = {'p2': 'bit', 'p4': 'sprite', 'p6': 'trow', 'lv': 'level'}
assert [len(GROUPS[g]) for g in GROUPS] == [3, 2, 2, 4], {g: len(v) for g, v in GROUPS.items()}
incomment, code = False, []
for l in L0:
    s = l.strip()
    if incomment:
        code.append(False)
        if '*/' in s:
            incomment = False
        continue
    if s.startswith('/*'):
        code.append(False)
        incomment = '*/' not in s
        continue
    code.append(True)
allw = [i for i, l in enumerate(L0) if code[i] and re.search(r'\bwork\b', l) and 's32 work;' not in l]
covered = sorted(sum(GROUPS.values(), []))
assert allw == covered, (allw, covered)
DECL = L0.index('    s32 work;')
JLOOP = find(r'work = \*\(s16 \*\)')[0]


def build(name, split, level_block=True):
    L = list(L0)
    for g in split:
        for ln in GROUPS[g]:
            L[ln] = re.sub(r'\bwork\b', NAME[g], L[ln])
    decls = [f'    s32 {NAME[g]};' for g in split if not (g in ('lv', 'p2') and level_block)]
    if 'lv' in split and level_block:
        L[JLOOP] = '        s32 level = ' + L[JLOOP].split('= ', 1)[1]
    if 'p2' in split and level_block:
        k = GROUPS['p2'][0]
        assert L[k - 1].rstrip().endswith('< 3) {'), L[k - 1]
        L[k - 1] = L[k - 1] + '\n            s32 bit;'
    if set(split) == set(GROUPS):
        # drop `work` and its comment block
        start = DECL
        while not L[start - 1].lstrip().startswith('/*'):
            start -= 1
        L[start - 1:DECL + 1] = decls
    else:
        L[DECL + 1:DECL + 1] = decls
    open(f'tmp/c21c9/{name}.c', 'w', newline='').write('\n'.join(L))
    return f'tmp/c21c9/{name}.c'


out = [build('w_spl_' + g, [g]) for g in GROUPS]
out.append(build('w_spl_all', list(GROUPS)))
out.append(build('w_spl_all_fnscope', list(GROUPS), level_block=False))
print(' '.join(out))
