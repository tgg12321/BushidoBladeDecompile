#!/usr/bin/env python3
"""title_mv_exec2: what would move the sign-extend qty out of $v1?

Replays block 2 of local-alloc and asks what extra quantity / hard-reg liveness
is required to push qty 0 (the sign-extended var_s0) to $a1, which is where
target holds it.
"""
import json
from pathlib import Path

ROWS = json.loads(
    Path('tmp/ra_solver_work/main.local.json').read_text())['func_800858D0']
BLK = [r for r in ROWS if r['blk'] == 2]
FIXED = {0, 1, 26, 27, 28, 29, 31, 67}


def floor_log2(x):
    return x.bit_length() - 1 if x > 0 else 0


def pri(r):
    return int((float(floor_log2(r['refs']) * r['refs']) /
                (r['death'] - r['birth'])) * 10000)


print('block 2 quantities (allocation order is by pri desc, tie -> lower qty):')
for r in sorted(BLK, key=lambda r: (-pri(r), r['qty'])):
    print('  qty %-2d refs=%-3d birth=%-3d death=%-3d span=%-3d pri=%-6d got=$%d'
          % (r['qty'], r['refs'], r['birth'], r['death'],
             r['death'] - r['birth'], pri(r), r['got']))

sx = [r for r in BLK if r['got'] == 3][0]
print('\nsign-extend qty = qty %d, span [%d,%d), pri %d, got $v1'
      % (sx['qty'], sx['birth'], sx['death'], pri(sx)))


def replay(extra=(), hard_live=()):
    """extra: (refs, birth, death) tuples for hypothetical added quantities.
       hard_live: (reg, birth, death) for hard regs live in the block."""
    rows = [dict(r) for r in BLK]
    for i, (refs, b, d) in enumerate(extra):
        rows.append({'qty': 900 + i, 'refs': refs, 'birth': b, 'death': d,
                     'got': None, 'pass': 'main'})
    live = {}
    for reg, b, d in hard_live:
        for i in range(b, d + 1):
            live.setdefault(i, set()).add(reg)
    out = {}
    for r in sorted(rows, key=lambda r: (-pri(r), r['qty'])):
        used = set(FIXED)
        for i in range(r['birth'], r['death']):
            used |= live.get(i, set())
        got = next(x for x in range(32) if x not in used)
        out[r['qty']] = got
        for i in range(r['birth'], r['death'] + 1):
            live.setdefault(i, set()).add(got)
    return out


base = replay()
print('\nbaseline replay: %s   (sign-extend -> $%d)' % (base, base[sx['qty']]))

print('\nwhat it takes to move the sign-extend:')
cases = [
    ('one extra qty overlapping it (refs 8, span 10)',
     [(8, 10, 20)], []),
    ('  + hard $a0 live across its range (target hoists the call arg)',
     [(8, 10, 20)], [(4, 6, 38)]),
    ('hard $a0 live across its range ALONE',
     [], [(4, 6, 38)]),
]
for label, extra, hard in cases:
    r = replay(extra, hard)
    print('  %-58s sign-extend -> $%d %s'
          % (label, r[sx['qty']], '<-- TARGET ($a1)' if r[sx['qty']] == 5 else ''))
