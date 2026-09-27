#!/usr/bin/env python3
"""usage (WSL, repo root): python3 tmp/func_8001A820/alloc.py <candidate.c> <tag>
Builds full.i via mk.py-style substitution, runs the INSTRUMENTED cc1 (tools/gcc-2.7.2/cc1)
with BB2_ALLOC_DEBUG and -dg, and prints the func_8001A820 allocation order lines plus
a pseudo->source-var map from the .greg dump (reg/v with decl names are not available, so
we print the first RTL set of each pseudo)."""
import os, re, subprocess, sys
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm
from engine import buildconfig as cfg

cand, tag = sys.argv[1], sys.argv[2]
out = Path(f'tmp/func_8001A820/build_{tag}')
out.mkdir(parents=True, exist_ok=True)
base = Path('src/code6cac.c').read_text()
body = Path(cand).read_text()
(out / 'full.c').write_text(inlineasm.substitute_body(base, 'func_8001A820', body))
subprocess.run(f"{cfg.CPP} {cfg.CPP_FLAGS} {cfg.CPP_DEFS} {out}/full.c > {out}/in.i", shell=True, check=True)
env = dict(os.environ, BB2_ALLOC_DEBUG='1')
if len(sys.argv) > 3:
    env['BB2_FINDREG_DEBUG'] = sys.argv[3]
r = subprocess.run(f"cd {out} && ../../../tools/gcc-2.7.2/cc1 {cfg.CC_FLAGS} -dg -dl in.i -o in.s",
                   shell=True, env=env, capture_output=True, text=True)
lines = [l for l in r.stderr.splitlines() if 'func=func_8001A820' in l]
fr, on = [], False
for l in r.stderr.splitlines():
    if l.startswith('FINDREGDBG func='):
        on = 'func_8001A820' in l
    if on and l.startswith('FINDREGDBG'):
        fr.append(l)
if len(sys.argv) > 3:
    print('\n'.join(fr))
    sys.exit(0)
(out / 'alloc.txt').write_text('\n'.join(lines) + '\n')
for l in lines:
    print(l.replace('ALLOCDBG func=func_8001A820 ', ''))
