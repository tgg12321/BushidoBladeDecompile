"""Allocator facts for the i/level/j question.
usage (WSL, after dump.sh <name> -dl -dg): python3 tmp/c21c/greg.py <name>...
Pseudo identities come from the .lreg dump (pre-allocation); dispositions from the .greg dump."""
import re, sys


def fn(path):
    txt = open(path).read()
    m = re.search(r'\n;; Function func_8006C21C\n(.*?)(\n;; Function |\Z)', txt, re.S)
    return m.group(1)


for name in sys.argv[1:]:
    lreg = fn(f'tmp/c21c/out/{name}.i.lreg')
    greg = fn(f'tmp/c21c/out/{name}.i.greg')
    disp = {}
    dm = re.search(r';; Register dispositions:\n(.*?)\n\n', greg, re.S)
    for a, b in re.findall(r'(\d+) in (\d+)', dm.group(1) if dm else ''):
        disp[int(a)] = int(b)
    lev = set(int(x) for x in re.findall(r'\(set \(reg/v:SI (\d+)\)\s*\(sign_extend:SI \(mem', lreg))
    cmp5 = set(int(x) for x in re.findall(r'\(ne:?S?I? ?\(reg/v:SI (\d+)\)\s*\(reg:SI \d+\)\)', lreg))
    ctr = {}
    for r, k in re.findall(r'\(lt:SI \(reg/v:SI (\d+)\)\s*\(const_int (\d+)\)\)', lreg):
        ctr.setdefault(int(r), set()).add(int(k))
    refs = {}
    for r, n in re.findall(r'\nRegister (\d+) used (\d+) times', lreg):
        refs[int(r)] = int(n)
    print(f'== {name}')
    for r in sorted(lev | cmp5 | set(ctr)):
        where = disp.get(r, 'STACK (not allocated)')
        tags = []
        if r in lev: tags.append('set from lh (level)')
        if r in cmp5: tags.append('compared with the 5 constant')
        if r in ctr: tags.append('loop test < ' + '/'.join(map(str, sorted(ctr[r]))))
        print(f'  pseudo {r}: hard reg {where}  refs {refs.get(r, "?")}  [{"; ".join(tags)}]')
