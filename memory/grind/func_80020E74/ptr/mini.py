"""mini.py <variant.c> [...]: compile a mini TU (prelude + variant funcs) as stem code6cac_tu2,
compare every function defined in it against build/src/code6cac_tu2.o. Run from repo root in WSL."""
import sys, re
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score

PRE = Path('tmp/func_80020E74/a6/prelude.h').read_text()
REF = 'build/src/code6cac_tu2.o'
for v in sys.argv[1:]:
    body = Path(v).read_text()
    tu = Path('tmp/func_80020E74/a6/obj') / (Path(v).stem + '.c')
    tu.parent.mkdir(parents=True, exist_ok=True)
    tu.write_text(PRE + body, newline='\n') if False else open(tu, 'w', newline='\n').write(PRE + body)
    out = str(tu.with_suffix('.o'))
    try:
        pipeline.build_c_object('code6cac_tu2', out, cheat_overrides={'src_override': str(tu)})
    except Exception as e:
        print(v, 'BUILD FAIL', str(e)[-800:]); continue
    fns = re.findall(r'^\w[\w\s\*]*?\b(func_\w+)\s*\([^;]*\)\s*\{', body, re.M)
    res = []
    for fn in fns:
        a = score.normalized_insns(REF, fn)
        b = score.normalized_insns(out, fn)
        s = score.score_func(out, REF, fn).get('score')
        res.append(f"{fn}={s}{'' if a==b else '*'}({len(b)}/{len(a)})")
        if '-v' in sys.argv[0:1] or Path(v).with_suffix('.show').exists():
            print('\n'.join(b))
    print(Path(v).name, ' '.join(res))
