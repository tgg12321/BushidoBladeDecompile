#!/usr/bin/env python3
"""SetDrawMode dtd/tw narrow-variable variants of candidate.c -> tmp/c21c9/d_*.c"""
import re
base = open('memory/grind/func_8006C21C/candidate.c', newline='').read()
DECL = '    s32 work;\n'
SDM = 'SetDrawMode(arg0[7], 1, 0, func_8006E480((s32)s.header, mode), 0);'
SDM8 = 'SetDrawMode(arg0[7], 1, 0, 0x40, 0);'
assert base.count(SDM) == 3 and base.count(SDM8) == 1
INIT = '    mode = 0;\n'
V = {}
def mk(name, decl, init, calls, call8=None, extra=None):
    s = base.replace(DECL, DECL + decl).replace(INIT, INIT + init, 1)
    parts = s.split(SDM)
    out = parts[0]
    for k, p in enumerate(parts[1:]):
        out += calls[k] + p
    if call8: out = out.replace(SDM8, call8)
    if extra:
        for a, b in extra: out = out.replace(a, b)
    V[name] = out
F = 'func_8006E480((s32)s.header, mode)'
def c(dtd, tw): return 'SetDrawMode(arg0[7], 1, %s, %s, %s);' % (dtd, F, tw)
for ty in ('s16', 'u16', 's8'):
    mk(f'dtd_all_{ty}', f'    {ty} dtd;\n', '    dtd = 0;\n', [c('dtd', '0')] * 3)
    mk(f'dtd_all8_{ty}', f'    {ty} dtd;\n', '    dtd = 0;\n', [c('dtd', '0')] * 3,
       'SetDrawMode(arg0[7], 1, dtd, 0x40, 0);')
    mk(f'dtdtw_{ty}', f'    {ty} dtd;\n    {ty} tw;\n', '    dtd = 0;\n    tw = 0;\n', [c('dtd', 'tw')] * 3)
    mk(f'dtdtw8_{ty}', f'    {ty} dtd;\n    {ty} tw;\n', '    dtd = 0;\n    tw = 0;\n', [c('dtd', 'tw')] * 3,
       'SetDrawMode(arg0[7], 1, dtd, 0x40, tw);')
for k, s in V.items():
    open(f'tmp/c21c9/d_{k}.c', 'w', newline='').write(s)
print(' '.join(f'tmp/c21c9/d_{k}.c' for k in V))
