"""Second pass over the 36 bodies: for every byte-offset / cast ACCESS, extract (base expression,
offset, access type). Also lists cast views without offsets and pointer locals initialised
from casts/globals, so each base can be traced to its object.
usage (repo root): python memory/grind/judge-decl-cleanup/scan-2026-09-30/scan2.py"""
import re
import sys
import os
import sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from span import _span  # noqa: E402
from collections import defaultdict

funcs = [l.strip() for l in open(os.path.join(HERE, 'final2_funcs.txt'), encoding='utf-8') if l.strip()]
files = ['src/code6cac.c', 'src/code6cac_b_tu2.c', 'src/code6cac_tu2.c', 'src/code6cac_c2.c']
ACC = re.compile(r'\*\s*\(\s*(\w+)\s*\*\s*\)\s*\(\s*(?:\(\s*u8\s*\*\s*\)\s*)?(&?\s*[\w\->.\[\]]+)\s*([+-])\s*(0x[0-9A-Fa-f]+|\d+)\s*\)')
ACC0 = re.compile(r'\*\s*\(\s*(\w+)\s*\*\s*\)\s*(&?[\w\->.\[\]]+)')
PTRDECL = re.compile(r'\b((?:u8|s8|s16|u16|s32|u32|void|[A-Z]\w*)\s*\*+)\s*(\w+)\s*(?:=\s*([^;]+))?;')
ASSIGN = re.compile(r'^\s*(\w+)\s*=\s*([^;]*(?:&D_|\(\s*\w+\s*\*\s*\)|0x1F80)[^;]*);')


def strip_comments(s):
    return re.sub(r'/\*.*?\*/', ' ', s, flags=re.S)


for fn in funcs:
    for f in files:
        raw = open(f, encoding='utf-8').read()
        try:
            a, b = _span(raw, fn)
        except AssertionError:
            continue
        body = strip_comments(raw[a:b])
        body = re.sub(r'__asm__\s+volatile\s*\(.*?\);', '', body, flags=re.S)
        acc = defaultdict(set)
        for m in ACC.finditer(body):
            acc[m.group(2).replace(' ', '')].add(f'{m.group(3)}{m.group(4)}:{m.group(1)}')
        for m in ACC0.finditer(body):
            if not re.match(r'\s*[+-]', body[m.end():m.end() + 3]):
                acc[m.group(2).replace(' ', '')].add(f'+0:{m.group(1)}')
        print(f'== {fn} ({f})')
        sig = raw[a:raw.index('{', a)].strip().replace('\n', ' ')
        print(f'   sig: {sig}')
        for m in PTRDECL.finditer(body):
            if m.group(3):
                print(f'   ptr: {m.group(1)} {m.group(2)} = {m.group(3).strip()[:90]}')
        for ln in body.split('\n'):
            m = ASSIGN.match(ln)
            if m:
                print(f'   set: {m.group(1)} = {m.group(2).strip()[:90]}')
        for base in sorted(acc):
            print(f'   acc: {base:<28} {" ".join(sorted(acc[base], key=lambda s: (int(s.split(":")[0].lstrip("+"), 0), s)))}')
        break
