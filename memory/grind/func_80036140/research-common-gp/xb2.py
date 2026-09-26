"""Scratch whole-file build of code6cac_b2_post.c with an optional include override dir and
optional candidate for func_80036140; score EVERY function in the file vs build/src reference.
usage: python3 tmp/func_80036140/xb.py <cand.c|-> [incdir] [--src FILE]
  --src FILE: use FILE as the full source (already edited) instead of substituting a candidate"""
import sys, subprocess, re
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm, score
from engine import buildconfig as cfg

STEM = 'code6cac_b2_post'
FUNC = 'func_80036140'
comm = []
_a = sys.argv[1:]
if '--comm' in _a:
    i = _a.index('--comm'); comm = _a[i + 1].split(','); del _a[i:i + 2]
args = [a for a in _a if a not in ('--g8', '--gate')]
src_file = None
if '--src' in args:
    i = args.index('--src'); src_file = args[i + 1]; del args[i:i + 2]
cand = args[0] if args else '-'
inc = args[1] if len(args) > 1 else None
wd = Path("tmp/research36140/xb/_o"); wd.mkdir(parents=True, exist_ok=True)
if src_file:
    text = Path(src_file).read_text()
else:
    text = Path(f'src/{STEM}.c').read_text()
    if cand != '-':
        text = inlineasm.substitute_body(text, FUNC, Path(cand).read_text())
(wd / f'{STEM}.c').write_text(text)
incflag = f'-I{inc} ' if inc else ''
out_o = wd / f'{STEM}.o'
stages = [f'{cfg.CPP} {incflag}{cfg.CPP_FLAGS} -Isrc {cfg.CPP_DEFS} {wd}/{STEM}.c',
          f'{cfg.CC1} {cfg.CC_FLAGS_GP if "--g8" in sys.argv else cfg.CC_FLAGS}', cfg.PROLOGUE_FIX]
if comm:
    pass
stages += [(f'python3 tmp/research36140/maspsx_comm/maspsx.py {cfg.MASPSX_FLAGS} --comm-syms=tmp/research36140/maspsx_comm_syms.txt' if '--gate' in sys.argv else f'{cfg.MASPSX} {cfg.MASPSX_FLAGS}'), cfg.MULTU_PAD, f'{cfg.AS} {cfg.AS_FLAGS} -o {out_o}']
cmd = ' | '.join(stages)
r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
if r.returncode:
    print(r.stderr[-3000:]); sys.exit(1)
ref = f'build/src/{STEM}.o'
funcs = score._o_func_table(ref)
bad = 0
for f in funcs:
    try:
        res = score.score_func(str(out_o), ref, f)
        s = res['score']
    except KeyError:
        s = 'MISSING'
    if s != 0:
        bad += 1
        print(f'{f}: {s}')
print(f'{len(funcs)} funcs, {bad} nonzero')
# byte-exact check of .text for non-target funcs
