#!/usr/bin/env python3
"""Orphan-USE census: compile every src/*.c with -dc, list per-function orphan
(use (reg N)) insns planted by combine, and the pseudo's references in the
pre-combine (.flow) dump so the eliminated computation can be read."""
import os, re, subprocess, sys, pathlib, json
sys.path.insert(0, '.')
from engine import buildconfig as B

out = pathlib.Path('tmp/c21c9/census'); out.mkdir(parents=True, exist_ok=True)
defs = ' '.join(B.CPP_DEFS) if isinstance(B.CPP_DEFS, (list, tuple)) else B.CPP_DEFS
res = {}
files = sys.argv[1:] or sorted(str(p) for p in pathlib.Path('src').glob('*.c'))
for f in files:
    stem = pathlib.Path(f).stem
    ii = out / f'{stem}.i'
    r = subprocess.run(f'{B.CPP} {B.CPP_FLAGS} -Isrc {defs} {f} > {ii}', shell=True,
                       capture_output=True, text=True)
    if r.returncode:
        print(stem, 'CPP FAIL', r.stderr[-300:]); continue
    r = subprocess.run(f'{B.CC1} {B.CC_FLAGS} -df -dc {ii} -o {out}/{stem}.s', shell=True,
                       capture_output=True, text=True)
    if r.returncode:
        print(stem, 'CC1 FAIL', r.stderr[-300:]); continue
    comb = open(str(ii) + '.combine').read()
    flow = open(str(ii) + '.flow').read()
    fsecs = dict(re.findall(r'\n;; Function (\S+)\n(.*?)(?=\n;; Function |\Z)', flow, re.S))
    for fn, sec in re.findall(r'\n;; Function (\S+)\n(.*?)(?=\n;; Function |\Z)', comb, re.S):
        regs = re.findall(r'\(insn \d+ \d+ \d+ \(use \(reg(?:/v)?:(\w+) (\d+)\)\)', sec)
        if not regs:
            continue
        info = []
        fl = fsecs.get(fn, '')
        insns = re.split(r'\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )', fl)
        for mode, n in regs:
            refs = [i.split('\n')[0][:60] + ' | ' + ' '.join(i.split())[:260]
                    for i in insns if re.search(r'\(reg(?:/v)?:\w+ %s\)' % n, i)]
            info.append({'reg': n, 'mode': mode, 'flow_refs': refs[:4]})
        res[f'{stem}:{fn}'] = info
        print(f'{stem}:{fn} orphans={len(regs)}')
json.dump(res, open(out / 'census.json', 'w'), indent=1)
