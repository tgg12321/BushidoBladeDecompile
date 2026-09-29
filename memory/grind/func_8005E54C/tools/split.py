#!/usr/bin/env python3
"""split.py <in.c> <out.c> <V...> : give the listed values of the shared `i` their own s16 local.
Values (line ranges in the d1/r11 body, 1-based inclusive) are located by anchors, not fixed numbers."""
import re, sys
src = open(sys.argv[1], newline='\n').read().split('\n')
want = set(sys.argv[3:])


def find(pred, start=0):
    for n in range(start, len(src)):
        if pred(src[n]):
            return n
    raise SystemExit('anchor not found')


# region anchors
r1 = find(lambda l: l.startswith('    for (i = 0; i < 2; i++)'))
r2 = find(lambda l: l.startswith('    for (i = 0; i < D_8009BD38.unk10 + 3; i++)'), r1 + 1)
r3 = find(lambda l: l.startswith('    for (i = 0; i < D_8009BD38.unk10 + 3; i++)'), r2 + 1)
r4 = find(lambda l: l.strip() == 'i = y;', r3 + 1)
r5 = find(lambda l: l.startswith('    for (i = 0; i < D_8009BD38.unk10 + 3; i++)'), r4 + 1)
end = find(lambda l: l.startswith('    s.header = &D_8009B398[0];'), r5 + 1)
ranges = {'V1': (r1, r2), 'V2': (r2, r3), 'V3': (r3, r4), 'V4': (r4, r5), 'V5': (r5, end)}
names = {'V1': 'i1', 'V2': 'i2', 'V3': 'i3', 'V4': 'base', 'V5': 'i5'}
for v in sorted(want):
    a, b = ranges[v]
    for n in range(a, b):
        src[n] = re.sub(r'\bi\b', names[v], src[n])
decl = find(lambda l: l == '    s16 i;')
for v in sorted(want, reverse=True):
    src.insert(decl + 1, '    s16 %s;' % names[v])
open(sys.argv[2], 'w', newline='\n').write('\n'.join(src))
