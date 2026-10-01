import sys, json
sys.path.insert(0, '.')
from engine import completion
d = json.load(open('tools/canonical_asm_regions.json'))['functions']
t = open(sys.argv[1], encoding='utf-8').read()
for f in sys.argv[2:]:
    h = completion.region_hashes(t, f)
    old = d[f]['sha256']
    print(f, 'SAME' if h == old else 'CHANGED', [i for i,(a,b) in enumerate(zip(h,old)) if a!=b], len(h), len(old))
    if h != old: print('  new', h)
