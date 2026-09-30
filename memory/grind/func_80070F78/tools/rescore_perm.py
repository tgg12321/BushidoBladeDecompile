"""Extract func_80070F78's definition from each permuter output's source.c into <out>/<name>.c
(prefixed with the candidate's extern block) for engine re-scoring with sc.py --mini."""
import sys
from pathlib import Path

d = Path(sys.argv[1])
out = Path(sys.argv[2])
out.mkdir(exist_ok=True)
head = ('extern s16 D_800A3540[];\nextern s16 D_800A3544[];\nextern u8 D_8009BC38[];\n'
        'extern s16 D_800A3594[];\nextern u8 D_800A3562;\n')
names = []
for o in sorted(d.glob('output-*')):
    src = (o / 'source.c').read_text(errors='replace')
    i = src.rfind('\nvoid func_80070F78(')
    if i < 0:
        continue
    p = out / f'{o.name}.c'
    p.write_text(head + src[i + 1:], newline='\n')
    names.append('tmp/func_80070F78/' + p.as_posix())
print(' '.join(names))
