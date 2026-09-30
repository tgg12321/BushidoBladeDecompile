"""Sandbox against a MODIFIED copy of src/text1b_tu1c.c (e.g. the aggregate merge), without
touching src/. usage (WSL, repo root): python3 tmp/f65800/msbx.py <tu.c> <candidate.c|-> [funcs...]
Scores func_80065800 (candidate substituted) and any extra funcs listed, against build/."""
import sys, json
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm, cheats, pipeline, score

tu, cand = sys.argv[1], sys.argv[2]
DIFF = '--diff' in sys.argv
funcs = [a for a in sys.argv[3:] if a != '--diff'] or ['func_80065800']
stem = 'text1b_tu1c'
wd = Path('tmp/sandbox_m')
wd.mkdir(parents=True, exist_ok=True)
base = Path(tu).read_text(encoding='utf-8')
text = base
if cand != '-':
    text = inlineasm.substitute_body(base, 'func_80065800', Path(cand).read_text(encoding='utf-8'))
ov = cheats.empty_overrides(str(wd / 'cfg'))
src_ovr = str(wd / 'src' / f'{stem}.c')
Path(src_ovr).parent.mkdir(parents=True, exist_ok=True)
inlineasm.write_stripped(stem, src_ovr, text)
ov['src_override'] = src_ovr
out_o = str(wd / f'{stem}.o')
pipeline.build_c_object(stem, out_o, cheat_overrides=ov)
tot = 0
for f in funcs:
    try:
        r = score.score_func(out_o, f'build/src/{stem}.o', f)
        print(f"{f:20s} score {r['score']:4d}  {r.get('build_insns')}/{r.get('target_insns')}")
        tot += r['score']
        if DIFF and r['score']:
            sys.path.insert(0, 'engine')
            from engine.cli import _print_insn_diff
            _print_insn_diff(score.insn_diff(out_o, f'build/src/{stem}.o', f))
    except KeyError as e:
        print(f, 'MISSING', e)
print('total', tot)
