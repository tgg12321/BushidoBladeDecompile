#!/usr/bin/env python3
"""Second family round: SL-value chain-extenders with NON-constant bases (so cse
cannot fold them before flow), placed in every other block and at the loop
edges, each alone and together with the SR chain-extender that fixes the SR seat
(fam/sr_chain_ior.c).  usage (WSL, repo root): python3 tmp/f8b488s4/mkfam2.py"""
import os
D = os.path.dirname(os.path.abspath(__file__))
base = open(D + '/split.c').read()
F = ' /* FAKE */'


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


def fscope(s, v):
    s = one(s, '            u16 %s;\n' % v, '')
    return one(s, '    s32 bSetAll;\n', '    s32 bSetAll;\n    u16 %s;\n' % v)


SR_IOR = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((sr_rate | smode) << 6);'
SR_IOR_CH = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode) + sr_rate - sr_rate) << 6);' + F
X = ' + sl_rate - sl_rate'
sites = {
    # block: (old, new)
    'ar_ior': ('adsr | ((ar_rate | amode) << 8);', 'adsr | ((ar_rate | amode)' + X + ') << 8);' + F),
    'dr_ior': ('adsr | (dr_rate << 4);', 'adsr | ((dr_rate' + X + ') << 4);' + F),
    'sr_ior': ('adsr | ((sr_rate | smode) << 6);', 'adsr | (((sr_rate | smode)' + X + ') << 6);' + F),
    'rr_ior': ('adsr | (rr_rate | rmode);', 'adsr | ((rr_rate | rmode)' + X + ');' + F),
    'pitch': ('= attr->pitch;', '= attr->pitch' + X + ';' + F),
    'pos': ('pos = voice * 8;', 'pos = voice * 8' + X + ';' + F),
    'step': ('voice < 24; voice++) {', 'voice < 24; voice = voice' + X + ' + 1) {' + F),
    'adsr2': ('*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = attr->adsr2;',
              '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = attr->adsr2' + X + ';' + F),
    'tail': ('    v = 1;\n', '    v = voice' + X + ' - 23;' + F + '\n'),
}
out = {}
s0 = fscope(base, 'sl_rate')
for k, (o, n) in sites.items():
    v = one(s0, o, n)
    out['sl2_' + k] = v
    if k == 'sr_ior':
        out['both2_' + k] = one(v, 'adsr | (((sr_rate | smode)' + X + ') << 6);' + F,
                                'adsr | (((sr_rate | smode) + sr_rate - sr_rate' + X + ') << 6);' + F)
    else:
        out['both2_' + k] = one(v, SR_IOR, SR_IOR_CH)
os.makedirs(D + '/fam2', exist_ok=True)
names = []
for k, v in sorted(out.items()):
    open('%s/fam2/%s.c' % (D, k), 'w', newline='\n').write(v)
    names.append('tmp/f8b488s4/fam2/%s.c' % k)
open(D + '/fam2/list.txt', 'w', newline='\n').write('\n'.join(names) + '\n')
print(len(names), 'variants')
