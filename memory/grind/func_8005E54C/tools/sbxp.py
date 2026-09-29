#!/usr/bin/env python3
"""MEASUREMENT tool: sandbox-score func_8005E54C with a candidate AND a TU-wide patch
(e.g. a declaration change elsewhere in src/text1b.c), in a private workdir.
Mirrors engine.sandbox.sandbox_score (disable=all, strip cheat-asm). Never touches src/ or build/.
Usage (WSL, repo root, venv): python3 tmp/func_8005E54C/sbxp.py <candidate.c> [patch.py] [--diff]
patch.py must define patch(text) -> text."""
import sys, json, runpy
from pathlib import Path
sys.path.insert(0, '.')
from engine import cheats, pipeline, score, inlineasm
from engine import cli as CLI

func, stem = 'func_8005E54C', 'text1b'
cand = sys.argv[1]
patch = sys.argv[2] if len(sys.argv) > 2 and not sys.argv[2].startswith('--') else None
wd = Path('tmp/func_8005E54C/sbxp')
ov = cheats.empty_overrides(str(wd / 'cfg'))
base = Path(f'src/{stem}.c').read_text(encoding='utf-8')
if patch:
    base = runpy.run_path(patch)['patch'](base)
text = inlineasm.substitute_body(base, func, Path(cand).read_text(encoding='utf-8'))
src_ovr = str(wd / 'src' / f'{stem}.c')
Path(src_ovr).parent.mkdir(parents=True, exist_ok=True)
inlineasm.write_stripped(stem, src_ovr, text)
ov['src_override'] = src_ovr
out_o = str(wd / f'{stem}.o')
pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
r = score.score_func(out_o, f'build/src/{stem}.o', func)
print(json.dumps(r))
if '--diff' in sys.argv:
    CLI._print_insn_diff(score.insn_diff(out_o, f'build/src/{stem}.o', func))
