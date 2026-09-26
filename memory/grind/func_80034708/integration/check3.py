import sys, json, subprocess
sys.path.insert(0, '.')
from engine import score
B = 'tmp/func_80034708/integ/build'
s = score.score_func(f'{B}/code6cac_b3.o', 'build/src/code6cac_b.o', 'func_80034708')
print('func_80034708', json.dumps({k: s[k] for k in ('score', 'target_insns', 'build_insns')}))
d = score.insn_diff(f'{B}/code6cac_b3.o', 'build/src/code6cac_b.o', 'func_80034708')
for h in d['hunks']:
    print('  @', h['class'], h['target'], h['built'])
sys.path.insert(0, 'tools')
import objdiff
a, b, same, changed, missing = objdiff.compare('build/src/code6cac_b.o', f'{B}/code6cac_b3_post.o')
print('tail present in b3_post:', sorted(set(b)))
print('tail changed:', [f for f in changed])
for f in changed:
    import difflib
    print('\n'.join(difflib.unified_diff(a[f], b[f], lineterm='', n=0)))
