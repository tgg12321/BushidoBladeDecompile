"""gen.py BASE OUTPREFIX < spec.py : spec defines VARIANTS = {name: [(old,new[,count]),...]}"""
import sys
from pathlib import Path
base = Path(sys.argv[1]).read_text()
prefix = sys.argv[2]
ns = {}
exec(sys.stdin.read(), ns)
outs = []
for name, reps in ns['VARIANTS'].items():
    t = base
    for r in reps:
        old, new = r[0], r[1]
        n = r[2] if len(r) > 2 else 1
        c = t.count(old)
        assert c == n, (name, c, n, old[:70])
        t = t.replace(old, new)
    p = f'{prefix}_{name}.c'
    Path(p).write_text(t, newline='\n')
    outs.append(p)
print(' '.join('tmp/func_80070F78/' + o for o in outs))
