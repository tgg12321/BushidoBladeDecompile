"""Score every function of the CD-module objects in a scratch tree vs the base tree's b2_post.o.
usage: python3 tmp/func_80036140/scoretree.py <name> [--diff func]"""
import sys, json
sys.path.insert(0, '.')
from pathlib import Path
from engine import score
T = Path('tmp/func_80036140') / sys.argv[1]
ref = 'tmp/func_80036140/base/build/src/code6cac_b2_post.o'
score._symtab()['D_80101E58'] = 0x80101E58
res, where = {}, {}
for stem in ('code6cac_b2_post', 'code6cac_b4', 'code6cac_b4_post', 'code6cac_b5', 'code6cac_b5_post'):
    o = T / 'build/src' / f'{stem}.o'
    if not o.exists():
        continue
    for f in score._o_func_table(str(o)):
        where[f] = str(o)
        try:
            res[f] = score.score_func(str(o), ref, f)['score']
        except Exception as e:
            res[f] = f'ERR {e}'[:60]
nz = {f: s for f, s in res.items() if s != 0}
print(f'{len(res)} funcs; nonzero: {nz or "none"}')
if '--diff' in sys.argv:
    f = sys.argv[sys.argv.index('--diff') + 1]
    d = score.insn_diff(where[f], ref, f)
    for h in d.get('hunks', []):
        print(json.dumps(h))
