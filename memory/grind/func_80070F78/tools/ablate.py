"""ablate.py base.c outprefix keep1,keep2 ... : variants keeping only the given oN locals (others inlined as i * 3)."""
import sys, re
from pathlib import Path

ALL = ['o1', 'o2', 'o3', 'o4', 'o5', 'o6', 'o7', 'o8', 'o9', 'o11', 'o12', 'o13', 'o14']


def make(t, keep):
    fk = [o for o in ALL if o in keep and o != 'o1']
    t = re.sub(r'    s32 o2[^;]*;\n', ('    s32 ' + ', '.join(fk) + ';\n') if fk else '', t)
    for o in ALL:
        if o in keep:
            continue
        t = re.sub(r'\n\s*(s32 )?' + o + r' = i \* 3;\n', '\n', t)
        t = t.replace('(' + o + ' = i * 3)', '(i * 3)')
        t = re.sub(r'\b' + o + r'\b', '(i * 3)', t)
    t = t.replace('D_800A3560[(i * 3)', 'D_800A3560[i * 3')
    return t


if __name__ == '__main__':
    base = Path(sys.argv[1]).read_text()
    out = []
    for spec in sys.argv[3:]:
        keep = spec.split(',')
        p = f'{sys.argv[2]}_{"_".join(keep)}.c'
        Path(p).write_text(make(base, keep), newline='\n')
        out.append('tmp/func_80070F78/' + p)
    print(' '.join(out))
