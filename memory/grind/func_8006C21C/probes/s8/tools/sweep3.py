"""Natural narrow position/flag locals: which zero-valued s16 reads orphan at zero cost."""
import os, itertools
src = open('tmp/c21c/base.c').read()
os.makedirs('tmp/c21c/sw3', exist_ok=True)
lines = src.split('\n')
# 1-based line numbers of the sites in base.c
Y = [63, 79, 102, 117]
X = [64, 101]
SEMI = [66, 87, 107, 116]
names = []


def make(name, ty, var, sites, repl_from, decl_init_line=60):
    L = list(lines)
    for s in sites:
        assert repl_from in L[s - 1], (name, s, L[s - 1])
        L[s - 1] = L[s - 1].replace(repl_from, '= ' + var + ';')
    # declare + init at top of body (before first statement, line 60 is 's.ot_idx = 10;')
    i = L.index('    s32 *arg0_unused;') if '    s32 *arg0_unused;' in L else None
    body_start = next(k for k, l in enumerate(L) if l.startswith('void func_8006C21C('))
    L.insert(body_start + 1, f'    {ty} {var};')
    first_stmt = next(k for k in range(body_start + 2, len(L)) if L[k].strip().startswith('s.ot_idx'))
    L.insert(first_stmt, f'    {var} = 0;')
    open(f'tmp/c21c/sw3/{name}.c', 'w', newline='\n').write('\n'.join(L))
    names.append(f'tmp/c21c/sw3/{name}.c')


for ty in ['s16', 'u16', 's8', 'u8']:
    make(f'y_all_{ty}', ty, 'ypos', Y, '= 0;')
    make(f'y_late_{ty}', ty, 'ypos', Y[2:], '= 0;')
    make(f'x_all_{ty}', ty, 'xpos', X, '= 0;')
    make(f'semi_all_{ty}', ty, 'semi', SEMI, '= 0;')
    make(f'semi_late_{ty}', ty, 'semi', SEMI[1:], '= 0;')
open('tmp/c21c/sw3/list.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))
