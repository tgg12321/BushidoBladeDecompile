# fdiff.py <stem> <func>: side-by-side of the tucheck build vs build/src for one function (changed rows only + context)
import sys, difflib
sys.path.insert(0, '.')
from engine import score
stem, f = sys.argv[1], sys.argv[2]
t = score.normalized_insns('build/src/%s.o' % stem, f)
o = score.normalized_insns('tmp/tucheck/%s/%s.o' % (stem, stem), f)
sm = difflib.SequenceMatcher(None, t, o, autojunk=False)
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        if i2 - i1 > 4:
            for k in range(i1, i1 + 2):
                print('   %4d %-34s|' % (k, t[k]))
            print('   ...')
            for k in range(i2 - 2, i2):
                print('   %4d %-34s|' % (k, t[k]))
        else:
            for k in range(i1, i2):
                print('   %4d %-34s|' % (k, t[k]))
        continue
    n = max(i2 - i1, j2 - j1)
    for k in range(n):
        a = t[i1 + k] if i1 + k < i2 else ''
        b = o[j1 + k] if j1 + k < j2 else ''
        print(' * %4s %-34s| %s' % (i1 + k if i1 + k < i2 else '', a, b))
