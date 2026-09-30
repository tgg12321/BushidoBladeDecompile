import re, sys
# bindiff.py: differing word addresses between tmp/c8dc2/tree/build/bb2.exe and the original, mapped to map symbols
a = open('tmp/c8dc2/tree/build/bb2.exe', 'rb').read()
b = open('disc/SLUS_006.63', 'rb').read()
print('sizes', len(a), len(b))
syms = []
for l in open('tmp/c8dc2/tree/build/bb2.map', errors='replace'):
    m = re.match(r'\s+0x([0-9a-f]{8})\s+(\S+)\s*$', l)
    if m:
        syms.append((int(m.group(1), 16), m.group(2)))
syms.sort()
import bisect
keys = [s[0] for s in syms]
diffs = [i for i in range(0x800, min(len(a), len(b)), 4) if a[i:i+4] != b[i:i+4]]
print('differing words', len(diffs))
seen = {}
for i in diffs:
    addr = 0x80010000 + i - 0x800
    k = bisect.bisect_right(keys, addr) - 1
    name = syms[k][1] if k >= 0 else '?'
    seen.setdefault(name, []).append(addr)
for n, v in list(seen.items())[:30]:
    print(n, len(v), ' '.join('%08x' % x for x in v[:6]))
