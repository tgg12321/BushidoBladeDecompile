"""Completeness scan of the 36 changed bodies (laneH, from tmp/rev-tables-r5/scan.py).
Flags, per source line (comments stripped): pointer casts `(T *)`, cast dereferences `*(T *)(`,
byte-offset arithmetic `+ 0x…` / `- 0x…` / `+ N` inside an address, scalar-& indexing `(&X)[` /
`&X +`, and the practice-table byte names. Output: one block per function, every hit with its
source line number. usage: python memory/grind/judge-decl-cleanup/scan-2026-09-30/scan.py > scan.txt (from the repo root)"""
import re
import sys
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from span import _span  # noqa: E402

funcs = [l.strip() for l in open(os.path.join(HERE, 'final2_funcs.txt'), encoding='utf-8') if l.strip()]
files = ['src/code6cac.c', 'src/code6cac_b_tu2.c', 'src/code6cac_tu2.c', 'src/code6cac_c2.c']
PATS = [
    ('cast', re.compile(r'\(\s*(?:const\s+)?(?:u8|s8|s16|u16|s32|u32|int|char|short|void|[A-Z]\w*|\w+_t)\s*\*+\s*\)')),
    ('offset', re.compile(r'[+-]\s*(?:0x[0-9A-Fa-f]+|\d+)\s*\)')),
    ('scalar&', re.compile(r'\(\s*&\s*\w+\s*\)\s*\[|&\s*D_\w+\s*[+-]|\(\s*u8\s*\*\s*\)\s*&')),
    ('table-name', re.compile(r'\bD_80101EC8\b|\bD_80101F\w+|\bD_801020\w+|\bD_801023\w+|\bD_801024\w+')),
]


def strip_comments(s):
    return re.sub(r'/\*.*?\*/', lambda m: re.sub(r'[^\n]', ' ', m.group(0)), s, flags=re.S)


total = 0
for fn in funcs:
    for f in files:
        raw = open(f, encoding='utf-8').read()
        try:
            a, b = _span(raw, fn)
        except AssertionError:
            continue
        line0 = raw.count('\n', 0, a) + 1
        body = strip_comments(raw[a:b])
        hits = []
        for k, ln in enumerate(body.split('\n')):
            kinds = [n for n, p in PATS if p.search(ln)]
            if 'asm' in ln or '__asm__' in ln:
                continue
            if kinds:
                hits.append((line0 + k, ','.join(kinds), ln.strip()))
        print(f'== {fn}  {f}  ({len(hits)} hits)')
        for n, kinds, ln in hits:
            print(f'   {f}:{n}  [{kinds}]  {ln}')
        total += len(hits)
        break
print(f'TOTAL {total} hits in {len(funcs)} functions')
