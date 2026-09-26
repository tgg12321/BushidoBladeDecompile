"""Census: in every src/*.c, compiled exactly as the build does (cpp|cc1, pre-maspsx), find load/store
instructions whose operand is `SYM+N` (or `SYM-N`) where SYM is in sdata_syms.txt and the enclosing
function is in sdata_funcs.txt (and SYM not excluded for it) -- i.e. every site where the current maspsx
applies %gp_rel to an OFFSET form of an injected extern. Prints each site.
usage (WSL, repo root, venv): python3 tmp/func_80036140/gp_off_census.py"""
import sys, re, subprocess
from pathlib import Path
sys.path.insert(0, '.')
from engine import buildconfig as cfg

sdata = {l.strip() for l in open('sdata_syms.txt') if l.strip() and not l.startswith('#')}
funcs = {l.strip() for l in open('sdata_funcs.txt') if l.strip() and not l.startswith('#')}
excl = {}
for l in open('sdata_exclude.txt'):
    if l.startswith('#') or ':' not in l:
        continue
    f, syms = l.split(':', 1)
    excl.setdefault(f.strip(), set()).update(s.strip() for s in syms.split(',') if s.strip())
mem = re.compile(r'^\s*(lb|lbu|lh|lhu|lw|lwl|lwr|sb|sh|sw|swl|swr)\s+\$\w+,\s*([A-Za-z_.$][\w.$]*)([+-]\d+)\s*$')
total = 0
for c in sorted(Path('src').glob('*.c')):
    stem = c.stem
    ccf = cfg.CC_FLAGS_GP if stem in cfg.GP_FILES else cfg.CC_FLAGS
    cmd = f'{cfg.CPP} {cfg.CPP_FLAGS} {cfg.CPP_DEFS} {c} | {cfg.CC1} {ccf}'
    r = subprocess.run(['bash', '-o', 'pipefail', '-c', cmd], capture_output=True, text=True)
    if r.returncode:
        print(f'!! {stem}: compile failed'); continue
    cur = None
    for ln in r.stdout.splitlines():
        m = re.match(r'^\s*\.ent\s+(\S+)', ln)
        if m:
            cur = m.group(1); continue
        m = mem.match(ln)
        if m and cur in funcs and m.group(2) in sdata and m.group(2) not in excl.get(cur, set()):
            total += 1
            print(f'{stem}:{cur}: {ln.strip()}')
print(f'TOTAL {total}')
