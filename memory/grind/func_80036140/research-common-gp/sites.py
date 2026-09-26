"""List every original access to given address ranges (from acc.json written by census.py).
usage: python3 tmp/research36140/sites.py 0x800A34F0:4 0x800A3588:8 ..."""
import json, sys
from collections import defaultdict
acc = json.load(open('tmp/research36140/acc.json'))
for spec in sys.argv[1:]:
    a0, n = spec.split(':'); a0 = int(a0, 16); n = int(n)
    print(f'== {a0:#x}..{a0 + n - 1:#x}')
    d = defaultdict(list)
    for f, a, m, w, op, va in acc:
        if a0 <= a < a0 + n:
            d[(a, m)].append(f'{f}:{op}')
    for (a, m), fs in sorted(d.items()):
        from collections import Counter
        c = Counter(fs)
        print(f'  {a:#x} +{a - a0} {m:4} ' + ', '.join(f'{k}x{v}' if v > 1 else k for k, v in c.items()))
