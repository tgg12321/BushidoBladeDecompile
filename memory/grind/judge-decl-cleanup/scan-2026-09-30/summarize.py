"""Group disposition.tsv by object: for each disposition class (a row may name several, joined by
' | '), list the functions and their source lines. Rows marked 'not debt: ...' are listed separately.
usage (repo root): python memory/grind/judge-decl-cleanup/scan-2026-09-30/summarize.py"""
import os
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
groups = defaultdict(lambda: defaultdict(list))
notdebt = defaultdict(list)
rows = 0
for ln in open(os.path.join(HERE, 'disposition.tsv'), encoding='utf-8'):
    if ln.startswith('function\t') or not ln.strip():
        continue
    fn, src, kinds, disp, text = ln.rstrip('\n').split('\t', 4)
    rows += 1
    line = src.rsplit(':', 1)[1] if ':' in src else src
    if disp.startswith('not debt'):
        notdebt[disp].append(f'{fn}:{line}')
        continue
    for d in disp.split(' | '):
        groups[d][fn].append(line)
print(f'{rows} hits; {sum(len(v) for v in notdebt.values())} not debt')
for d in sorted(groups, key=lambda k: (not k.startswith(('PracticeMenuRec', 'Rec44', 'D_800F5F68')), k)):
    fns = groups[d]
    print(f'\n{d}  ({sum(len(v) for v in fns.values())} hits)')
    for fn in sorted(fns):
        print(f'  {fn}: lines {", ".join(fns[fn])}')
print('\nnot debt:')
for d, v in notdebt.items():
    print(f'  {d}: {", ".join(v)}')
