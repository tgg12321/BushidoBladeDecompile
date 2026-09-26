"""Scratch whole-file build of code6cac_b2_post.c for func_80036940 modelling.
usage: python3 tmp/func_80036940/xb.py <cand.c> [--inc DIR] [--g8] [--comm a,b] [--b36140 FILE] [--diff] [--tag NAME]
Scores func_80036940 (and reports every other function in the file that is nonzero) vs build/src."""
import os, sys, subprocess, json
if os.name == 'nt':  # Windows python + bash -c eats the backslashes: 'tmp\x' -> 'tmpx' at the repo root
    sys.exit('xb.py must run under WSL (bash tools/wsl.sh ...), not Windows python')
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm, score
from engine import buildconfig as cfg

STEM = 'code6cac_b2_post'
a = sys.argv[1:]
def opt(name, default=None):
    if name in a:
        i = a.index(name); v = a[i + 1]; del a[i:i + 2]; return v
    return default
def flag(name):
    if name in a:
        a.remove(name); return True
    return False
inc = opt('--inc'); base = opt('--base', f'src/{STEM}.c'); comm = opt('--comm'); b36140 = opt('--b36140'); tag = opt('--tag', 'xb')
g8 = flag('--g8'); show = flag('--diff'); quiet = flag('--quiet')
cand = a[0]
wd = Path(f'tmp/func_80036940/_{tag}'); wd.mkdir(exist_ok=True)
text = Path(base).read_text()
text = inlineasm.substitute_body(text, 'func_80036940', Path(cand).read_text())
if b36140:
    text = inlineasm.substitute_body(text, 'func_80036140', Path(b36140).read_text())
(wd / f'{STEM}.c').write_text(text)
incflag = f'-I{inc} ' if inc else ''
out_o = wd / f'{STEM}.o'
stages = [f'{cfg.CPP} {incflag}{cfg.CPP_FLAGS} -Isrc {cfg.CPP_DEFS} {wd}/{STEM}.c',
          f'{cfg.CC1} {cfg.CC_FLAGS_GP if g8 else cfg.CC_FLAGS}', cfg.PROLOGUE_FIX]
if comm:
    stages.append(f'python3 memory/grind/func_80036140/integration/inject_comm.py {comm}')
stages += [f'{cfg.MASPSX} {cfg.MASPSX_FLAGS}', cfg.MULTU_PAD, f'{cfg.AS} {cfg.AS_FLAGS} -o {out_o}']
r = subprocess.run(['bash', '-o', 'pipefail', '-c', ' | '.join(stages)], capture_output=True, text=True)
if r.returncode:
    print(r.stderr[-3000:]); sys.exit(1)
ref = f'build/src/{STEM}.o'
score._symtab()['D_80101E58'] = 0x80101E58  # scratch: the E58-based record symbol
others = []
main = None
for f in score._o_func_table(ref):
    try:
        s = score.score_func(str(out_o), ref, f)['score']
    except KeyError:
        s = 'MISSING'
    if f == 'func_80036940':
        main = s
    elif s != 0:
        others.append(f'{f}={s}')
print(f'func_80036940 score={main}  others_nonzero={others or "none"}')
if show:
    d = score.insn_diff(str(out_o), ref, 'func_80036940')
    for h in d.get('hunks', []):
        if (h.get('cls') or h.get('class')) == 'not-scored':
            continue
        print(json.dumps(h))
