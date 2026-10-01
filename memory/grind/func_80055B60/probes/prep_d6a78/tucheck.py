# tucheck.py <stem> <modified-src.c> [inc-dir]: build the modified TU (faithful pipeline, src override,
# optional include dir ahead of include/) and score EVERY function in it against build/src/<stem>.o.
import sys, os
sys.path.insert(0, '.')
from engine import buildconfig as cfg
stem, srcp = sys.argv[1], sys.argv[2]
inc = sys.argv[3] if len(sys.argv) > 3 else None
if inc:
    cfg.CPP_FLAGS = '-I%s ' % inc + cfg.CPP_FLAGS
from engine import pipeline, score, cheats
wd = 'tmp/tucheck/%s' % stem
os.makedirs(wd + '/src', exist_ok=True)
dst = wd + '/src/%s.c' % stem
open(dst, 'w', newline='\n').write(open(srcp).read())
ov = cheats.empty_overrides(wd + '/cfg')
ov['src_override'] = dst
out = wd + '/%s.o' % stem
pipeline.build_c_object(stem, out, cheat_overrides=ov)
ref = 'build/src/%s.o' % stem
funcs = sorted(score._o_func_table(ref).keys())
bad = 0
for f in funcs:
    try:
        r = score.score_func(out, ref, f)
    except KeyError as e:
        print('MISSING', f); bad += 1; continue
    if r.get('score'):
        print('DIFF', f, r['score']); bad += 1
print('functions', len(funcs), 'nonzero', bad)
