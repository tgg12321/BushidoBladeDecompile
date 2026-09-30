"""Score candidate bodies for func_80070F78 like `sandbox --disable all
--candidate`, with the landing-time overrides applied to the COPY only:
  - sc_reps.SRC_REPS: literal replacements on the substituted text1b.c copy
    (field rename + the 0x800A3560 record merge across all consumers)
  - sdata_exclude.txt / sdata_syms.txt copies under tmp/func_80070F78/
usage: python3 tmp/func_80070F78/sc.py [--diff|--hunks] [--plain] [--mini]
          [--pre=path] [--funcs=f1,f2] cand.c [cand2.c ...]
--plain: no overrides (== engine sandbox). --mini: prefix TU (mkpre.py)."""
import sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score, inlineasm, cheats
from engine import buildconfig as cfg
from engine.cli import _print_insn_diff
import importlib.util
_s = importlib.util.spec_from_file_location('sc_reps', 'tmp/func_80070F78/sc_reps.py')
_m = importlib.util.module_from_spec(_s); _s.loader.exec_module(_m)

FUNC = 'func_80070F78'
STEM = 'text1b'
args = sys.argv[1:]
diff = '--diff' in args
hunks = '--hunks' in args
plain = '--plain' in args
mini = '--mini' in args
funcs = [a[8:] for a in args if a.startswith('--funcs=')]
funcs = funcs[0].split(',') if funcs else [FUNC]
cands = [a for a in args if not a.startswith('--')]
if '--g8' in args:
    cfg.GP_FILES = set(cfg.GP_FILES) | {'text1b'}
    cfg.MASPSX_FLAGS_GP = cfg.MASPSX_FLAGS_GP.replace('--sdata-exclude=sdata_exclude.txt', '--sdata-exclude=tmp/func_80070F78/sdata_exclude.txt').replace('--sdata-syms=sdata_syms.txt', '--sdata-syms=tmp/func_80070F78/sdata_syms.txt')
if not plain:
    cfg.MASPSX_FLAGS = cfg.MASPSX_FLAGS.replace('--sdata-exclude=sdata_exclude.txt', '--sdata-exclude=tmp/func_80070F78/sdata_exclude.txt')
    cfg.MASPSX_FLAGS = cfg.MASPSX_FLAGS.replace('--sdata-syms=sdata_syms.txt', '--sdata-syms=tmp/func_80070F78/sdata_syms.txt')
base = Path(f'src/{STEM}.c').read_text(encoding='utf-8')
for c in cands:
    wd = Path('tmp/func_80070F78/sc') / (Path(c).stem + ('_g8' if '--g8' in args else ''))
    ov = cheats.empty_overrides(str(wd / 'cfg'))
    body = Path(c).read_text(encoding='utf-8')
    if mini:
        prep = [a[6:] for a in args if a.startswith('--pre=')]
        prep = prep[0] if prep else 'tmp/func_80070F78/mini/pre.c'
        txt = Path(prep).read_text(encoding='utf-8') + chr(10) + body
    else:
        txt = inlineasm.substitute_body(base, FUNC, body)
        if not plain:
            txt = _m.apply(txt)
    src_ovr = str(wd / 'src' / f'{STEM}.c')
    Path(src_ovr).parent.mkdir(parents=True, exist_ok=True)
    n = inlineasm.write_stripped(STEM, src_ovr, txt)
    ov['src_override'] = src_ovr
    o = str(wd / f'{STEM}.o')
    try:
        pipeline.build_c_object(STEM, o, cheat_overrides=ov)
    except Exception as e:
        msg = str(e)
        lines = [l for l in msg.splitlines() if '.c:' in l][:3]
        print(f'{c}: BUILD FAILED {lines}')
        continue
    for f in funcs:
        r = score.score_func(o, f'build/src/{STEM}.o', f)
        print(f"{c} {f}: score {r['score']} insns {r['build_insns']}/{r['target_insns']} stripped={n}")
        if diff or hunks:
            d = score.insn_diff(o, f'build/src/{STEM}.o', f)
            if hunks:
                d['hunks'] = [h for h in d['hunks'] if h['class'] != 'not-scored']
            _print_insn_diff(d)
