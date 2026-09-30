#!/usr/bin/env python3
"""WSL: python3 tmp/f759d0/rtl.py <candidate.c> <tag>
Splice candidate into src/text1b.c in place of the INCLUDE_ASM line, truncate
after it, cpp, cc1 -da with the real text1b flags. Writes per-pass files for
func_800759D0 only into tmp/f759d0/rtl/<tag>/<pass>, plus in.s (func only)."""
import re, shlex, subprocess, sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from engine import buildconfig as cfg  # noqa

FUNC = 'func_800759D0'
cand, tag = sys.argv[1], sys.argv[2]
out = ROOT / 'tmp/f759d0/rtl' / tag
out.mkdir(parents=True, exist_ok=True)
for p in out.iterdir():
    p.unlink()
src = (ROOT / 'src/text1b.c').read_text()
line = 'INCLUDE_ASM("asm/funcs", %s);' % FUNC
i = src.index(line)
tu = src[:i] + (ROOT / cand).read_text() + '\n'
(out / 'tu.c').write_text(tu)
cmd = '%s %s %s tmp/f759d0/rtl/%s/tu.c' % (cfg.CPP, cfg.CPP_FLAGS, cfg.CPP_DEFS, tag)
r = subprocess.run(['bash', '-c', cmd], cwd=ROOT, capture_output=True, text=True)
(out / 'in.i').write_text(r.stdout)
flags = cfg.CC_FLAGS_GP if 'text1b' in cfg.GP_FILES else cfg.CC_FLAGS
if 'text1b' in cfg.NO_SR_FILES:
    flags += ' -fno-strength-reduce'
cmd = '%s %s -da in.i -o in.s' % (shlex.quote(str(ROOT / cfg.CC1)), flags)
r = subprocess.run(['bash', '-c', cmd], cwd=out, capture_output=True, text=True)
if r.stderr.strip():
    print(r.stderr[-1500:])
# keep only our function's section of each dump
for p in sorted(out.iterdir()):
    if not p.name.startswith('in.i.') and p.name != 'in.s':
        continue
    txt = p.read_text(errors='replace')
    if p.name == 'in.s':
        k = txt.find('\n%s:' % FUNC)
        body = txt[k:] if k >= 0 else txt
    else:
        k = txt.find(';; Function %s' % FUNC)
        body = txt[k:] if k >= 0 else ''
    (out / (p.name.replace('in.i.', '') if p.name != 'in.s' else 'asm.s')).write_text(body)
    p.unlink()
(out / 'in.i').unlink()
print('ok', sorted(x.name for x in out.iterdir()))
