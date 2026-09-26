import sys
sys.path.insert(0, '.')
from engine import score
o, ref, fn = sys.argv[1:4]
s = score.score_func(o, ref, fn)
print('SCORE', s['score'], s['build_insns'])
