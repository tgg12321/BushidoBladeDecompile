"""Simulate func_800720FC compiled in a TU that declares D_800A3578 as s16 and
D_800A35C8/D_800A35CA as s16 scalars: splice the candidate into a copy of
src/text1b.c, retype those declarations, compile with text1b's recipe, score."""
import re, sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline, score, inlineasm
cand = sys.argv[1]
base = Path('src/text1b.c').read_text(encoding='utf-8')
body = Path(cand).read_text(encoding='utf-8')
txt = inlineasm.substitute_body(base, 'func_800720FC', body)

txt = txt.replace('extern u16 D_800A3578;', 'extern s16 D_800A3578;')
# neutralize other users of the retyped names (their bodies are not scored)
out = Path('tmp/func_800720FC/rtu/src'); out.mkdir(parents=True, exist_ok=True)
p = out / 'text1b.c'
p.write_text(txt, encoding='utf-8', newline='\n')
o = 'tmp/func_800720FC/rtu/text1b.o'
try:
    pipeline.build_c_object('text1b', o, cheat_overrides={'src_override': str(p)})
except Exception as e:
    print('BUILD FAILED', e); sys.exit(1)
print(score.score_func(o, 'build/src/text1b.o', 'func_800720FC'))
