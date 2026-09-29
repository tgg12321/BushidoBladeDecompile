#!/usr/bin/env python3
"""Ruling 9 (i) receipts: one local per `cells` write (cells1..cells4), same statement list as the
landing body (tmp/c21c9/land_sb.c). Variants: all at function scope; innermost-block scope."""
import re
S = open('tmp/c21c9/land_sb.c', newline='').read()
W = '= s.header + 0xC;'
L = S.split('\n')
sites = [i for i, l in enumerate(L) if 'cells ' + W in l]
assert len(sites) == 4, sites
reads = [i for i, l in enumerate(L) if 's.table = cells;' in l]
assert len(reads) == 4 and all(r == w + 1 for r, w in zip(reads, sites)), (sites, reads)
DECL = L.index('    u8 *cells;')


def build(name, block):
    M = list(L)
    for k, (w, r) in enumerate(zip(sites, reads), 1):
        M[w] = M[w].replace('cells ' + W, f'cells{k} ' + W)
        M[r] = M[r].replace('s.table = cells;', f's.table = cells{k};')
    fn = [1, 3] if block else [1, 2, 3, 4]
    M[DECL] = '\n'.join(f'    u8 *cells{k};' for k in fn)
    if block:
        # innermost scope of writes 2 (phase-2 `if` body) and 4 (phase-4 `for` body):
        # insert the declaration as the first line of the enclosing compound statement
        for k in (2, 4):
            w = sites[k - 1]
            j = w
            while not M[j].rstrip().endswith('{'):
                j -= 1
            ind = re.match(r' *', M[j + 1]).group(0)
            M[j] = M[j] + f'\n{ind}u8 *cells{k};'
    out = '\n'.join(M)
    assert out.count('s.header + 0xC') == 4
    open(f'tmp/c21c9/{name}.c', 'w', newline='\n').write(out)
    return f'tmp/c21c9/{name}.c'


print(build('cells_per_write_fn', False), build('cells_per_write_block', True))
