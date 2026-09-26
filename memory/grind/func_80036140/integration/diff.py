import sys, json
sys.path.insert(0, '.')
from engine import score
d = score.insn_diff('tmp/func_80036140/_xb/code6cac_b2_post.o', 'build/src/code6cac_b2_post.o', sys.argv[1] if len(sys.argv) > 1 else 'func_80036140')
for h in d.get('hunks', []):
    if h.get('cls') == 'not-scored' or h.get('class') == 'not-scored':
        continue
    print(json.dumps(h))
print({k: v for k, v in d.items() if k != 'hunks'})
