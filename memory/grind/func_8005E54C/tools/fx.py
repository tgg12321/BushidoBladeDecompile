#!/usr/bin/env python3
"""Extract func_8005E54C's section from each RTL dump in a dump dir into <dir>/fn.<pass>."""
import sys, os, glob
d = sys.argv[1]
fn = sys.argv[2] if len(sys.argv) > 2 else 'func_8005E54C'
for p in glob.glob(os.path.join(d, 't.i.*')):
    txt = open(p, errors='replace').read()
    key = ';; Function %s' % fn
    i = txt.find(key)
    if i < 0:
        continue
    j = txt.find(';; Function ', i + len(key))
    out = os.path.join(d, 'fn.' + p.rsplit('.', 1)[1])
    open(out, 'w').write(txt[i:j if j > 0 else None])
    os.remove(p)
print('ok')
