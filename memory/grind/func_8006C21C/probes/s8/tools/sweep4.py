"""xpos/ypos (s16 = 0) at subsets of natural sites. Each site: (line, old, new)."""
import os, itertools
src = open('tmp/c21c/base.c').read()
os.makedirs('tmp/c21c/sw4', exist_ok=True)
lines = src.split('\n')
SITES = {
    'x1': (64, 's.x = 0;', 's.x = xpos;'),
    'x4': (101, 's.x = 0;', 's.x = xpos;'),
    'y1': (63, 's.y = 0;', 's.y = ypos;'),
    'y2': (79, 's.y = 0;', 's.y = ypos;'),
    'y4': (102, 's.y = 0;', 's.y = ypos;'),
    'y4i': (117, 's.y = 0;', 's.y = ypos;'),
    'x8': (154, 'x = 0;', 'x = xpos;'),
    'x2': (81, 's.x = pl * 280;', 's.x = xpos + pl * 280;'),
    'x4i': (121, 's.x = j ? 280 : 0;', 's.x = j ? xpos + 280 : xpos;'),
    'x6': (141, 'tile->x0 = tile_rec->x + j * 280;', 'tile->x0 = tile_rec->x + xpos + j * 280;'),
}
names = []


def make(name, sites):
    L = list(lines)
    usex = any(s.startswith('x') for s in sites)
    usey = any(s.startswith('y') for s in sites)
    for s in sites:
        ln, a, b = SITES[s]
        assert a in L[ln - 1], (s, L[ln - 1])
        L[ln - 1] = L[ln - 1].replace(a, b)
    body = next(k for k, l in enumerate(L) if l.startswith('void func_8006C21C('))
    decl = []
    init = []
    if usex:
        decl.append('    s16 xpos;'); init.append('    xpos = 0;')
    if usey:
        decl.append('    s16 ypos;'); init.append('    ypos = 0;')
    L[body + 1:body + 1] = decl
    first = next(k for k in range(body + 1, len(L)) if L[k].strip().startswith('s.ot_idx'))
    L[first:first] = init
    open(f'tmp/c21c/sw4/{name}.c', 'w', newline='\n').write('\n'.join(L))
    names.append(f'tmp/c21c/sw4/{name}.c')


singles = ['x4', 'y4', 'y4i', 'x8', 'x2', 'x4i', 'x6']
for s in singles:
    base = ['x1', s] if s.startswith('x') else ['y1', 'y2', s]
    make('s_' + s, base)
make('xy_a', ['x1', 'x4', 'y1', 'y2', 'y4'])
make('xy_b', ['x1', 'x4', 'x8', 'y1', 'y2', 'y4'])
make('xy_c', ['x1', 'x4', 'x8'])
make('xy_d', ['x1', 'x4', 'x8', 'x6'])
make('xy_e', ['x1', 'x4', 'x8', 'x2'])
make('xy_f', ['x1', 'x4', 'x8', 'x2', 'x4i', 'x6'])
make('xy_g', ['x1', 'x4', 'x8', 'x2', 'x4i', 'x6', 'y1', 'y2', 'y4', 'y4i'])
make('xy_h', ['x1', 'x4', 'x8', 'y1', 'y2', 'y4', 'y4i'])
open('tmp/c21c/sw4/list.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))

# --- round 2: semi sites on top of xy_a
SITES.update({
    's1': (66, 's.semi = 0;', 's.semi = semi;'),
    's2': (87, 's.semi = 0;', 's.semi = semi;'),
    's4': (107, 's.semi = 0;', 's.semi = semi;'),
    's4i': (116, 's.semi = 0;', 's.semi = semi;'),
})
_make = make
def make2(name, sites):
    _make(name, sites)
    p = f'tmp/c21c/sw4/{name}.c'
    t = open(p).read()
    if any(s.startswith('s') and s[1:2].isdigit() for s in sites):
        t = t.replace('void func_8006C21C(s32 *arg0) {\n', 'void func_8006C21C(s32 *arg0) {\n    s16 semi;\n', 1)
        t = t.replace('    xpos = 0;\n', '    xpos = 0;\n    semi = 0;\n', 1)
    open(p, 'w', newline='\n').write(t)
names.clear()
base_a = ['x1', 'x4', 'y1', 'y2', 'y4']
for extra in (['s1', 's4'], ['s1', 's2'], ['s1', 's4i'], ['s1', 's2', 's4'], ['s1', 's4', 's4i'], ['s1', 's2', 's4', 's4i']):
    make2('a_' + '_'.join(extra), base_a + extra)
open('tmp/c21c/sw4/list2.txt', 'w', newline='\n').write(' '.join(names))
print(len(names))
