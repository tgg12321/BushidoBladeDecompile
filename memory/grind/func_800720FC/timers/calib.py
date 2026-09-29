"""Q21 (1) / Q25 / Q24 harness for the D_800A35C8/CA per-file declaration.

usage: calib.py <name> <tu.c> <func> [<func> ...]

<tu.c> is a complete translation unit (prepared by variants.py). For it:
  - the build pipeline (text1b recipe) -> <out>/<name>.o, and the engine score
    of every listed function against build/src/text1b.o (0 = byte-identical);
  - the build's cc1 -S and cc1psx -S (tools/cc1psx_wrapper.sh) of the SAME
    preprocessed TU, before maspsx -> <out>/<name>.cc1.s / <name>.cc1psx.s,
    each function cut to <out>/<name>.<func>.{cc1,cc1psx}.s.
Writes <out>/<name>.json.
"""
import json
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, '.')
from engine import buildconfig as cfg, pipeline, score  # noqa: E402

OUT = Path('memory/grind/func_800720FC/timers/out')
name, tu = sys.argv[1], sys.argv[2]
funcs = sys.argv[3:]
OUT.mkdir(parents=True, exist_ok=True)

res = {'name': name, 'tu': tu, 'scores': {}}
o = OUT / f'{name}.o'
try:
    pipeline.build_c_object('text1b', str(o), cheat_overrides={'src_override': tu})
    for f in funcs:
        res['scores'][f] = score.score_func(str(o), 'build/src/text1b.o', f)
except Exception as e:  # a build failure is a result, not a crash
    res['build_error'] = str(e)[-400:]

i_file = OUT / f'{name}.i'
cpp = f"{cfg.CPP} {cfg.CPP_FLAGS} -Isrc {cfg.CPP_DEFS} {tu}"
subprocess.run(f"{cpp} > {i_file}", shell=True, check=True)
subprocess.run(f"{cfg.CC1} {cfg.CC_FLAGS} {i_file} -o {OUT / (name + '.cc1.s')}", shell=True, check=True)
psx_flags = "-O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -w -msoft-float"
subprocess.run(f"bash tools/cc1psx_wrapper.sh {psx_flags} < {i_file} | tr -d '\\r' > {OUT / (name + '.cc1psx.s')}",
               shell=True, check=True)
i_file.unlink()


def cut(path, func):
    lines, on = [], False
    for ln in Path(path).read_text().splitlines():
        if ln.startswith(func + ':'):
            on = True
        if on:
            lines.append(ln)
            if ln.strip().startswith('.end') and func in ln:
                break
    return '\n'.join(lines) + '\n'


for f in funcs:
    for comp in ('cc1', 'cc1psx'):
        (OUT / f'{name}.{f}.{comp}.s').write_text(cut(OUT / f'{name}.{comp}.s', f))
(OUT / f'{name}.cc1.s').unlink()
(OUT / f'{name}.cc1psx.s').unlink()
(OUT / f'{name}.json').write_text(json.dumps(res, indent=1))
print(json.dumps(res))
