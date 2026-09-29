"""mkpv.py LANDED OUT -- derive the one-variable-per-value spelling of the landed body:
temp -> ang (turn block) / hit (collision result); work -> amt (turn block) / rest (restitution).
Only declarations and identifiers change."""
import re
import sys

src = open(sys.argv[1], encoding='utf-8').read()
lines = src.split('\n')
out = []
split = None
for k, l in enumerate(lines):
    if 'temp = func_8005344C' in l:
        split = k
assert split is not None
decl_done = False
k = 0
while k < len(lines):
    l = lines[k]
    # replace the two commented declarations
    if l.strip().startswith('/* work holds') or l.strip().startswith('/* temp holds'):
        while not lines[k].strip().endswith('*/'):
            k += 1
        k += 1
        continue
    if l.strip() == 's32 work;':
        out.append('    s32 amt;')
        out.append('    s32 rest;')
        k += 1
        continue
    if l.strip() == 's32 temp;':
        out.append('    s32 ang;')
        out.append('    s32 hit;')
        k += 1
        continue
    if k < split:
        l = re.sub(r'\btemp\b', 'ang', l)
        l = re.sub(r'\bwork\b', 'amt', l)
    else:
        l = re.sub(r'\btemp\b', 'hit', l)
        l = re.sub(r'\bwork\b', 'rest', l)
    out.append(l)
    k += 1
open(sys.argv[2], 'w', encoding='utf-8', newline='\n').write('\n'.join(out))
