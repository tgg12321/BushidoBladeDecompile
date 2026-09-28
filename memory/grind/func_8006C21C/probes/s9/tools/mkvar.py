#!/usr/bin/env python3
"""Generate phase-8 / phase-2 s16-level variants of candidate.c into tmp/c21c9/v_*.c"""
import pathlib
base = open('memory/grind/func_8006C21C/candidate.c', newline='').read()
MEM8 = '*(s16 *)(D_800A34FC + j * 2 + 0x28)'
L8 = '        work = %s;\n        rec = &recs[work + 1];\n' % MEM8
assert L8 in base
DECL = '    s32 work;\n'
assert DECL in base
T5 = 'if (work == 5)'
assert base.count(T5) == 2
V = {}
def mk(name, l8, t1=T5, t2=T5, decl='    s16 lv;\n'):
    s = base.replace(L8, l8).replace(DECL, DECL + decl)
    i = s.index(T5); s = s[:i] + t1 + s[i + len(T5):]
    i = s.index(T5, i + len(t1)) if T5 in s[i + len(t1):] else -1
    if i >= 0: s = s[:i] + t2 + s[i + len(T5):]
    V[name] = s
mk('p1', '        lv = %s;\n        rec = &recs[lv + 1];\n        work = lv;\n' % MEM8)
mk('p2', '        lv = %s;\n        work = lv;\n        rec = &recs[lv + 1];\n' % MEM8)
mk('p3', '        lv = %s;\n        rec = &recs[lv + 1];\n' % MEM8, 'if (lv == 5)', 'if (lv == 5)')
mk('p4', '        work = lv = %s;\n        rec = &recs[lv + 1];\n' % MEM8)
mk('p5', '        lv = %s;\n        rec = &recs[lv + 1];\n        work = lv;\n' % MEM8, 'if (lv == 5)', T5)
mk('p6', '        lv = %s;\n        rec = &recs[lv + 1];\n        work = lv;\n' % MEM8, T5, 'if (lv == 5)')
mk('p7', '        lv = %s;\n        rec = recs + lv + 1;\n        work = lv;\n' % MEM8)
mk('p8', '        lv = %s;\n        work = lv + 1;\n        rec = &recs[work];\n        work = lv;\n' % MEM8)
for k, s in V.items():
    open(f'tmp/c21c9/v_{k}.c', 'w', newline='').write(s)
print(' '.join(f'tmp/c21c9/v_{k}.c' for k in V))
