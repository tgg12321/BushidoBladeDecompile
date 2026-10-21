"""tools/naming_wavefollow.py <wave> [extra files] -- run at the repo (or lane worktree) root.

Apply a wave's CONFIRM pairs to docs/engine, given rule files, and live
named_syms.txt comment lines (RESET / retired-name history notes untouched)."""
import csv, re, glob, sys
wave = sys.argv[1]
extra = sys.argv[2:]
rows = list(csv.DictReader(open(f'docs/naming/phase3/{wave}/func_manifest.csv', encoding='utf-8')))
pairs = [(r['current_name'], r['proposed_name']) for r in rows
         if r.get('verdict') == 'CONFIRM' and r.get('action') == 'RENAME']
SKIP = ('RESET 0x', 'RENAME 0x', 'retired name', 'preserved note')

def subl(s):
    for a, b in pairs:
        s = re.sub(r'\b%s\b' % re.escape(a), b, s)
    return s

def apply(path, history=False):
    t = open(path, encoding='utf-8', newline='').read()
    if history:
        n = '\n'.join(l if any(k in l for k in SKIP) else subl(l) for l in t.split('\n'))
    else:
        n = subl(t)
    if n != t:
        open(path, 'w', encoding='utf-8', newline='').write(n)
        print('changed', path)

REGISTRIES = ('named_syms.txt', 'symbol_addrs.txt')
for f in glob.glob('docs/engine/*.md') + [e for e in extra if e not in REGISTRIES]:
    apply(f)
for f in REGISTRIES:
    if f == 'named_syms.txt' or f in extra:
        apply(f, history=True)
