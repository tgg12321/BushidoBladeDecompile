#!/usr/bin/env python3
"""Frame/orphan probe for func_8006C21C.
usage: python3 tmp/c21c/orph.py cand.c [cand2.c ...]
For each candidate: splice into text1b.c, cpp, cc1 (project flags) with -dc,
print vars=, insn count, and the orphan (use (reg N)) insns in the .combine dump
for the function (with the preceding insn/label for context)."""
import os, re, subprocess, sys, pathlib
sys.path.insert(0, '.')
from engine import buildconfig as B
from engine import inlineasm

FUNC = 'func_8006C21C'
out = pathlib.Path('tmp/c21c/out'); out.mkdir(parents=True, exist_ok=True)
base = open('src/text1b.c', newline='').read()
defs = ' '.join(B.CPP_DEFS) if isinstance(B.CPP_DEFS, (list, tuple)) else B.CPP_DEFS

def run(cand, verbose):
    name = pathlib.Path(cand).stem
    body = open(cand, newline='').read()
    text = inlineasm.substitute_body(base, FUNC, body)
    tu = out / f'{name}_tu.c'
    open(tu, 'w', newline='').write(text)
    ii = out / f'{name}.i'
    r = subprocess.run(f'{B.CPP} {B.CPP_FLAGS} -Isrc {defs} {tu} > {ii}', shell=True,
                       capture_output=True, text=True)
    if r.returncode:
        print(name, 'CPP FAIL', r.stderr[-800:]); return
    s = out / f'{name}.s'
    r = subprocess.run(f'{B.CC1} {B.CC_FLAGS} -dc {ii} -o {s}', shell=True,
                       capture_output=True, text=True)
    if r.returncode or not s.exists():
        print(name, 'CC1 FAIL', r.stderr[-1500:]); return
    lines = open(s).read().splitlines()
    st = next(i for i, l in enumerate(lines) if l.startswith(FUNC + ':'))
    en = next(i for i in range(st, len(lines)) if lines[i].strip().startswith('.end') and FUNC in lines[i])
    fn = lines[st:en + 1]
    frame = next((l.strip() for l in fn if '.frame' in l), '')
    ninsn = sum(1 for l in fn if l.startswith('\t') and not l.strip().startswith('.') and not l.strip().startswith('#'))
    open(out / f'{name}.fn.s', 'w').write('\n'.join(fn) + '\n')
    comb = str(ii) + '.combine'
    txt = open(comb).read()
    # isolate function section
    m = re.search(r'\n;; Function ' + FUNC + r'\n(.*?)(\n;; Function |\Z)', txt, re.S)
    sec = m.group(1) if m else ''
    insns = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', sec)
    orphans = []
    for k, blk in enumerate(insns):
        if re.match(r'\(insn \d+ \d+ \d+ \(use \(reg', blk):
            prev = insns[k - 1].split('\n')[0][:80] if k else ''
            orphans.append((blk.split('\n')[0][:90], prev))
    vm = re.search(r'vars= (\d+)', frame)
    diff = ''
    ref = out / 'base.fn.s'
    if ref.exists() and name != 'base':
        import difflib
        def norm(ls):
            r = []
            for l in ls:
                s = l.strip()
                if not s or s.startswith('.') or s.startswith('#'):
                    continue
                s = re.sub(r'\.L\d+', 'L', s)
                s = re.sub(r'-?\d+\(\$sp\)', 'N($sp)', s)
                r.append(s)
            return r
        a = norm(open(ref).read().splitlines()); b = norm(fn)
        sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
        nd = sum(max(i2 - i1, j2 - j1) for t, i1, i2, j1, j2 in sm.get_opcodes() if t != 'equal')
        pos = sum(1 for x, y in zip(a, b) if x != y) + abs(len(a) - len(b))
        diff = f' codediff={nd} posdiff={pos}'
    print(f'{name:40s} vars={vm.group(1) if vm else "?"} lines={ninsn} orphans={len(orphans)}{diff}')
    if verbose:
        for o, p in orphans:
            print('    ', o, '   <after>', p)
    for f in out.glob(f'{name}.i.*'):
        if not str(f).endswith('.combine'):
            f.unlink()

verbose = '-v' in sys.argv
for c in [a for a in sys.argv[1:] if a != '-v']:
    run(c, verbose)
