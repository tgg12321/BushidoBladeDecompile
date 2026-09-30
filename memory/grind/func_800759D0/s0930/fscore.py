import sys
# fscore.py <stem> <func>...: engine score of scratch-tree object vs main's build/ object
sys.path.insert(0, '.')
from engine import score
stem = sys.argv[1]
for f in sys.argv[2:]:
    try:
        r = score.score_func(f'tmp/c8dc/tree/build/src/{stem}.o', f'build/src/{stem}.o', f)
        print(f, r.get('score'), r.get('build_insns'), r.get('target_insns'))
    except Exception as e:
        print(f, 'ERR', e)
