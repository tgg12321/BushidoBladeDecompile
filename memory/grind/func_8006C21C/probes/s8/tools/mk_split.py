"""R11 (D)(4) ablations for the i/level variable of cand10: split each value into its own local."""
import re
NL = chr(10)
L0 = open('tmp/c21c/cand10.c').read().split(NL)
GROUPS = {'p2': [85, 87, 88], 'p4': [114, 115], 'p6': [136, 137], 'lv': [159, 160, 163, 198]}


def build(name, split):
    L = list(L0)
    decl = []
    for g in split:
        v = 'i_' + g
        for ln in GROUPS[g]:
            L[ln - 1] = re.sub(r'\bi\b', v, L[ln - 1])
        decl.append(f'    s32 {v};')
    assert L[54] == '    s32 i;', L[54]
    if set(split) == set(GROUPS):
        L[54:55] = decl
    else:
        L[55:55] = decl
    open(f'tmp/c21c/{name}.c', 'w', newline=NL).write(NL.join(L))
    return f'tmp/c21c/{name}.c'


out = [build('spl_' + g, [g]) for g in GROUPS]
out.append(build('spl_all', list(GROUPS)))
print(' '.join(out))
