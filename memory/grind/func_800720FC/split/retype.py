"""Build a copy of src/text1b.c with D_800A3578 declared s16 (no other change)
and compare the whole object's .text/.data/.rodata bytes + relocations to
build/src/text1b.o."""
import subprocess, sys
sys.path.insert(0, '.')
from pathlib import Path
from engine import pipeline
txt = Path('src/text1b.c').read_text(encoding='utf-8')
n = txt.count('extern u16 D_800A3578;')
txt = txt.replace('extern u16 D_800A3578;', 'extern s16 D_800A3578;')
out = Path('tmp/func_800720FC/retype/src'); out.mkdir(parents=True, exist_ok=True)
p = out / 'text1b.c'; p.write_text(txt, encoding='utf-8', newline='\n')
o = 'tmp/func_800720FC/retype/text1b.o'
pipeline.build_c_object('text1b', o, cheat_overrides={'src_override': str(p)})
def dump(obj):
    r = subprocess.run(['mipsel-linux-gnu-objdump', '-s', '-r', obj], capture_output=True, text=True)
    return [l for l in r.stdout.splitlines() if not l.startswith(obj) and 'file format' not in l]
a, b = dump(o), dump('build/src/text1b.o')
print('decls retyped:', n, 'identical:', a == b, len(a), len(b))
if a != b:
    import difflib
    for l in list(difflib.unified_diff(b, a, lineterm=''))[:40]: print(l)
