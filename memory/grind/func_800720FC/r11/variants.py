"""Ruling 11 (D)(4) spellings for func_800720FC's shared `cells`.

Values (one write each; no read can see two of them):
  V1 line 78   cells = s.header + 0x18;  read at 80 and 88 (first block, if-arm)
  V2 line 102  cells = s.header + 0xC;   read at 103
  V3 line 152  ...                        read at 153
  V4 line 198  ...                        read at 199
  V5 line 222  ... (grid inner loop body) read at 223
  V6 line 230  ...                        read at 231
Writes tmp/func_800720FC/r11/<name>.c:
  split_all  - one fresh local per value, each declared at the innermost scope
               enclosing its write (V1: the if-block; V5: the inner loop body;
               V2/V3/V4/V6: function scope)
  only_Vk    - value k alone gets its own local (same scope rule), the rest stay
               on the shared `cells`
  struct_rw  - structural respelling: no local at all, each read uses
               `s.header + K` directly (V1's loop read re-derives it too)
"""
import re
from pathlib import Path

SRC = Path('memory/grind/func_800720FC/landing_body.c')
OUT = Path('tmp/func_800720FC/r11')
OUT.mkdir(parents=True, exist_ok=True)
base = SRC.read_text(encoding='utf-8')
lines = base.split('\n')

# 1-based line numbers of each value's write and reads (verified below)
VALUES = {
    1: (78, [80, 88]),
    2: (102, [103]),
    3: (152, [153]),
    4: (198, [199]),
    5: (222, [223]),
    6: (230, [231]),
}
for k, (w, rs) in VALUES.items():
    assert re.match(r'\s*cells = s\.header \+ 0x(18|C);', lines[w - 1]), (k, lines[w - 1])
    for r in rs:
        assert 'cells' in lines[r - 1] and 's.cells' in lines[r - 1], (k, lines[r - 1])
# the function-scope declaration of `cells` (with its comment block)
decl_start = next(i for i, l in enumerate(lines) if l.startswith('    s32 cells; /* several values'))
decl_end = decl_start
while not lines[decl_end].rstrip().endswith('*/'):
    decl_end += 1


def rename(ls, k, name):
    w, rs = VALUES[k]
    ls[w - 1] = re.sub(r'\bcells\b(?= = s\.header)', name, ls[w - 1])
    for r in rs:
        ls[r - 1] = re.sub(r'(?<!s\.)\bcells\b', name, ls[r - 1])


def local_decl(ls, k, name):
    """Declare `name` at the innermost scope enclosing value k's write."""
    w = VALUES[k][0]
    if k == 1:   # first statement block after `if ((D_800A3578 & 0xFF) == 0) {`
        i = next(j for j in range(w - 1, 0, -1) if ls[j].strip() == 'if ((D_800A3578 & 0xFF) == 0) {')
        ls[i] = ls[i] + '\n        s32 %s;\n' % name
    elif k == 5:  # inner loop body
        i = next(j for j in range(w - 1, 0, -1) if 'for (j = 0, row = i * 2' in ls[j])
        ls[i] = ls[i] + '\n            s32 %s;\n' % name
    else:
        ls[decl_start - 1] = ls[decl_start - 1] + '\n    s32 %s;' % name


def emit(name, ls):
    (OUT / f'{name}.c').write_text('\n'.join(ls), encoding='utf-8', newline='\n')


def drop_shared(ls):
    for i in range(decl_start, decl_end + 1):
        ls[i] = None
    return [l for l in ls if l is not None]


# split_all
ls = list(lines)
for k in VALUES:
    rename(ls, k, 'cells%d' % k)
for k in sorted(VALUES, reverse=True):
    local_decl(ls, k, 'cells%d' % k)
emit('split_all', drop_shared(ls))
# only_Vk
for k in VALUES:
    ls = list(lines)
    rename(ls, k, 'cells%d' % k)
    local_decl(ls, k, 'cells%d' % k)
    emit('only_V%d' % k, ls)
# struct_rw: no variable; reads use s.header + K
ls = list(lines)
for k, (w, rs) in VALUES.items():
    expr = '(sheets[4] + 0x18)' if k == 1 else '(s.header + 0xC)'
    ls[w - 1] = None
    for r in rs:
        ls[r - 1] = re.sub(r'(?<!s\.)\bcells\b', expr, ls[r - 1])
emit('struct_rw', drop_shared(ls))
print(sorted(p.name for p in OUT.iterdir()))
