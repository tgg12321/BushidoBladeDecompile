"""Scratch whole-file build: compile a full source FILE on src/<stem>'s exact recipe, optionally with an
include-override dir searched before include/, and score EVERY function vs build/src/<stem>.o.
usage: python3 tmp/func_80036140/xb.py <stem> <src-file> [--inc DIR] [--keep OUT.o] [--diff FUNC]"""
import sys, subprocess
from pathlib import Path
sys.path.insert(0, '.')
from engine import score, pipeline

a = sys.argv[1:]
inc = None; keep = None; dfunc = None
if '--inc' in a:
    i = a.index('--inc'); inc = a[i + 1]; del a[i:i + 2]
if '--keep' in a:
    i = a.index('--keep'); keep = a[i + 1]; del a[i:i + 2]
if '--diff' in a:
    i = a.index('--diff'); dfunc = a[i + 1]; del a[i:i + 2]
stem, src = a[0], a[1]
wd = Path('tmp/func_80036140/_xb'); wd.mkdir(parents=True, exist_ok=True)
out_o = keep or str(wd / f'{stem}.o')
cmd = pipeline.c_pipeline_cmd(stem, out_o, {'src_override': src})
import os
if os.environ.get('XB_SDF'):
    cmd = cmd.replace('--sdata-funcs=sdata_funcs.txt', '--sdata-funcs=' + os.environ['XB_SDF'])
if inc:
    cmd = cmd.replace(' -Iinclude ', f' -I{inc} -Iinclude ', 1)
r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
if r.returncode:
    print(r.stderr[-3000:]); sys.exit(1)
ref = f'build/src/{stem}.o'
funcs = score._o_func_table(ref)
bad = 0
for f in funcs:
    try:
        s = score.score_func(out_o, ref, f)['score']
    except KeyError:
        s = 'MISSING'
    if s != 0:
        bad += 1
    print(f'  {f}: {s}')
print(f'{stem}: {len(funcs)} funcs, {bad} nonzero')
if dfunc:
    res = score.score_func(out_o, ref, dfunc)
    for k in ('diff', 'hunks'):
        if k in res:
            print(res[k])
