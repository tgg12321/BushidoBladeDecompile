import re, glob, os
tot = reuse = 0
for f in sorted(glob.glob('memory/grind/func_800187F4/rejected/v3_review/*_res.txt')):
    rows = [(m.group(1), int(m.group(2))) for l in open(f) for m in [re.match(r'(\S+)\s+diff=(\d+)', l)] if m]
    r = [t for t, d in rows if 'c7' in t]
    rz = [t for t, d in rows if d == 0 and t in r]
    pv = [(t, d) for t, d in rows if t not in r]
    print(os.path.basename(f), len(rows), 'reuse', len(r), 'reuse-zeros', len(rz), 'pv', len(pv),
          'pv-zeros', sum(1 for t, d in pv if d == 0), 'pv-best', min(d for t, d in pv))
    tot += len(pv); reuse += len(r)
print('per-value total', tot, 'reuse bodies', reuse)
