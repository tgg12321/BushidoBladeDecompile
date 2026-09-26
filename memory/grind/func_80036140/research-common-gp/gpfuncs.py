"""gp-relative symbols per function, in address order, for the functions of a src file.
usage: python3 tmp/research36140/gpfuncs.py src/code6cac_b2_post.c"""
import json, re, sys
from collections import defaultdict
acc = json.load(open('tmp/research36140/acc.json'))
names = {}
for ln in open('tmp/research36140/nm.txt'):
    p = ln.split()
    if len(p) == 3:
        names.setdefault(int(p[0], 16), p[2])
gp = defaultdict(set); lui = defaultdict(set); va0 = {}
for f, a, m, w, op, va in acc:
    va0[f] = min(va0.get(f, 1 << 40), va)
    (gp if m == 'gp' else lui)[f].add(a)
src = open(sys.argv[1]).read()
funcs = re.findall(r'^(?:INCLUDE_ASM\("asm/funcs", (\w+)\)|[A-Za-z_][\w \*]*?\b(\w+)\([^;]*\)\s*\{)', src, re.M)
funcs = [a or b for a, b in funcs]
for f in funcs:
    syms = sorted(gp.get(f, ()))
    print(f'{f:28} gp: ' + ', '.join(f'{a:#x}' for a in syms))
