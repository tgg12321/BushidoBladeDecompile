"""Generate candidate variants from a base by literal replacements.
usage: python gen.py base.c outdir name:old=>new[;;old=>new] ...
Each spec is read from a python file variants.py: VARIANTS = {name: [(old, new), ...]}"""
import sys, importlib.util
from pathlib import Path

base = Path(sys.argv[1]).read_text(encoding='utf-8')
spec_file = sys.argv[2]
outdir = Path(sys.argv[3]); outdir.mkdir(parents=True, exist_ok=True)
spec = importlib.util.spec_from_file_location('v', spec_file)
m = importlib.util.module_from_spec(spec); spec.loader.exec_module(m)
names = []
for name, reps in m.VARIANTS.items():
    t = base
    for old, new in reps:
        n = t.count(old)
        if n == 0:
            print(f'{name}: MISSING {old!r}', file=sys.stderr); sys.exit(1)
        t = t.replace(old, new)
    p = outdir / f'{name}.c'
    p.write_bytes(t.encode('utf-8'))
    names.append(str(p).replace('\\', '/'))
print(','.join(names))
