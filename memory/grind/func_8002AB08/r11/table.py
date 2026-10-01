# table.py: allocation of each reused local (landing body) and of each value pseudo in the twins.
# Pseudo map (landing body, locals declared in the player-loop block): 86 alt, 87 temp1, 88 temp2,
# 90 idx, 91 dx, 92 dy, 93 dz, 95 temp3, 103 work; a twin's fresh values are 106.. in roles.py order.
import re
R = {2: 'v0', 3: 'v1', 4: 'a0', 5: 'a1', 6: 'a2', 7: 'a3', 8: 't0', 9: 't1', 10: 't2', 11: 't3', 12: 't4',
     13: 't5', 14: 't6', 15: 't7', 16: 's0', 17: 's1', 18: 's2', 19: 's3', 20: 's4', 21: 's5', 22: 's6',
     23: 's7', 24: 't8', 25: 't9', 30: 'fp', -1: 'mem'}


def info(tag, p):
    a = [l for l in open(f'dumps/{tag}/{tag}.alloc') if f' pseudo={p} ' in l]
    r = [l for l in open(f'dumps/{tag}/{tag}.regs') if l.startswith(f'Register {p} ')]
    hr = int(re.search(r'hardreg=(-?\d+)', a[0]).group(1)) if a else None
    calls = re.search(r'crosses (\d+) call', r[0]).group(1) if r and 'crosses' in r[0] else '0'
    blk = 'local' if r and 'in block' in r[0] else ''
    return f"{R.get(hr, hr) if a else 'local-alloc'} calls={calls} {blk}".strip()


cand = {86: 'alt', 87: 'temp1', 88: 'temp2', 90: 'idx', 91: 'dx', 92: 'dy', 93: 'dz', 95: 'temp3', 103: 'work'}
print("cand:", ", ".join(f"{n}({p})={info('cand', p)}" for p, n in cand.items()))
for v in ['dx', 'dy', 'dz', 'temp1', 'temp2', 'temp3', 'idx', 'alt', 'work', 'all']:
    s = open(f'fpv_{v}.c').read()
    decls = re.findall(r'\n *s32 (\w+_);', s)
    print(f"pv_{v}:", ", ".join(f"{d}({106 + k})={info('pv_' + v, 106 + k)}" for k, d in enumerate(decls)))
