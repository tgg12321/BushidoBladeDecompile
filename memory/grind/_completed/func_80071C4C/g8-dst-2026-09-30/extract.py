import re, sys
from pathlib import Path
W = Path('tmp/audit-2026-09-29/g8-dump71c4c')
for v in ('landed', 'inlined1'):
    for f in sorted((W / v).glob(f'{v}.i.*')):
        t = f.read_text(errors='replace')
        m = re.search(r'\n;; Function func_80071C4C\n', t)
        if not m: print('nofunc', f); continue
        e = t.find('\n;; Function ', m.end())
        (W / v / ('71c4c.' + f.name.split('.')[-1])).write_text(t[m.start():e if e > 0 else None])
    s = (W / v / 'frozen.s').read_text()
    a = s.index('\nfunc_80071C4C:'); e = s.index('.end\tfunc_80071C4C', a)
    (W / v / '71c4c.s').write_text(s[a:e])
