# mkfinal.py: build tmp/func_80055B60/final/{A,AB}/<path> = the base commit's files + landing A (D_80106A78 cluster)
# [+ landing B (func_80055B60)], and tmp/func_80055B60/final/base/<path>. Run from the repo root.
import json, os, subprocess, sys
sys.path.insert(0, '.')
sys.path.insert(0, 'tmp/func_80055B60')
BASE = open('tmp/func_80055B60/BASE').read().split('=')[1].strip()
F = 'tmp/func_80055B60/final'
D = 'tmp/func_80055B60/d6a'
PATHS = ['include/code6cac.h', 'src/code6cac_b_tu2.c', 'src/code6cac_b.c', 'src/code6cac_tu2.c', 'src/text1b.c',
         'named_syms.txt', 'undefined_syms_auto.txt', 'tools/canonical_asm_regions.json']

def show(p):
    return subprocess.run(['git', 'show', '%s:%s' % (BASE, p)], capture_output=True, check=True).stdout.decode('utf-8')

def put(tag, p, text):
    q = os.path.join(F, tag, p)
    os.makedirs(os.path.dirname(q), exist_ok=True)
    open(q, 'w', encoding='utf-8', newline='\n').write(text)

def rd(p):
    return open(p, encoding='utf-8', newline='').read()

base = {p: show(p) for p in PATHS}
for p in PATHS:
    put('base', p, base[p])

# ---- A: the D_80106A78 cluster
A = dict(base)
A['include/code6cac.h'] = rd(D + '/include/code6cac.h')
A['src/code6cac_b_tu2.c'] = rd(D + '/src/code6cac_b_tu2.c')
A['src/code6cac_b.c'] = rd(D + '/src/code6cac_b.c')
A['src/code6cac_tu2.c'] = rd(D + '/src/code6cac_tu2.c')
A['named_syms.txt'] = rd(D + '/named_syms.txt')
A['undefined_syms_auto.txt'] = rd(D + '/undefined_syms_auto.txt')
from engine import completion
j = json.loads(base['tools/canonical_asm_regions.json'])
j['functions']['func_800300B4']['sha256'] = completion.region_hashes(A['src/code6cac_b_tu2.c'], 'func_800300B4')
# keep the file's own formatting: replace only the one hash string
old = json.loads(base['tools/canonical_asm_regions.json'])['functions']['func_800300B4']['sha256']
new = j['functions']['func_800300B4']['sha256']
t = base['tools/canonical_asm_regions.json']
for a, b in zip(old, new):
    if a != b:
        assert t.count(a) == 1, a
        t = t.replace(a, b)
A['tools/canonical_asm_regions.json'] = t
for p in PATHS:
    put('A', p, A[p])

# ---- B on top of A: func_80055B60
import hdr, mksrc
AB = dict(A)
AB['include/code6cac.h'] = hdr.transform(A['include/code6cac.h'])
AB['src/text1b.c'] = mksrc.transform(A['src/text1b.c'], rd('tmp/func_80055B60/cand3.c'))
for p in PATHS:
    put('AB', p, AB[p])
# symbol files through syms_b.py on the written copies
subprocess.run([sys.executable, 'tmp/func_80055B60/syms_b.py', F + '/AB/undefined_syms_auto.txt', F + '/AB/named_syms.txt'], check=True)
print('final A / AB written under', F)
