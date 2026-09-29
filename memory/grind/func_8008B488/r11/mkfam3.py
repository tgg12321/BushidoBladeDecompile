#!/usr/bin/env python3
"""Third family round (layer-2 round-1 objection): a same-iteration WRITE of the SL
variable inside another block followed by a combine-foldable chain-extender READ there,
so the per-value SL pseudo is also live in that block (where $a0 may be held) without
being live around the loop.  sl_rate is declared in the loop body (the innermost scope
enclosing both blocks).  Each is split.c plus the named construct only, alone ("_n")
and with the SR chain-extender of fam/sr_chain_ior.c ("_c").
usage (WSL, repo root): python3 tmp/f8b488s4/mkfam3.py -> tmp/f8b488s4/fam3/*.c"""
import os
D = os.path.dirname(os.path.abspath(__file__))
base = open(D + '/split.c').read()
F = ' /* FAKE */'


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


s0 = one(base, '            u16 sl_rate;\n', '')
s0 = one(s0, '    for (voice = 0; voice < 24; voice++) {\n',
         '    for (voice = 0; voice < 24; voice++) {\n        u16 sl_rate;\n\n')
X = ' + sl_rate - sl_rate'
SR_AND = '            adsr &= 0x3F;\n'
SR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((sr_rate | smode) << 6);'
SR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode)' + X + ') << 6);' + F
SR_ST_XC = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode) + sr_rate - sr_rate' + X + ') << 6);' + F
SR_ST_C = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode) + sr_rate - sr_rate) << 6);' + F
DR_AND = '            adsr &= 0xFF0F;\n'
DR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (dr_rate << 4);'
DR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((dr_rate' + X + ') << 4);' + F
RR_AND = '            adsr &= 0xFFC0;\n'
RR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (rr_rate | rmode);'
RR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((rr_rate | rmode)' + X + ');' + F
out = {}
# SR block: the write sources, placed after the adsr mask (so $a0 = masked adsr is live)
for tag, src in (('load', 'attr->sl'), ('srrate', 'sr_rate'), ('smode', 'smode'),
                 ('adsr', 'adsr'), ('pos', 'pos'), ('rxx', '*(volatile u16 *)(_spu_RXX + (pos + 4) * 2)')):
    w = one(s0, SR_AND, SR_AND + '            sl_rate = %s;' % src + F + '\n')
    out['sr_w_%s_n' % tag] = one(w, SR_ST, SR_ST_X)
    out['sr_w_%s_c' % tag] = one(w, SR_ST, SR_ST_XC)
# the write BEFORE the mask (value live across the andi that sets $a0)
w = one(s0, '            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);\n            adsr &= 0x3F;\n',
        '            sl_rate = attr->sl;' + F + '\n            adsr = *(volatile u16 *)(_spu_RXX + (pos + 5) * 2);\n            adsr &= 0x3F;\n')
out['sr_wearly_load_n'] = one(w, SR_ST, SR_ST_X)
out['sr_wearly_load_c'] = one(w, SR_ST, SR_ST_XC)
# DR and RR blocks
for blk, AND, ST, STX in (('dr', DR_AND, DR_ST, DR_ST_X), ('rr', RR_AND, RR_ST, RR_ST_X)):
    w = one(s0, AND, AND + '            sl_rate = attr->sl;' + F + '\n')
    w = one(w, ST, STX)
    out['%s_w_load_n' % blk] = w
    out['%s_w_load_c' % blk] = one(w, SR_ST, SR_ST_C)
os.makedirs(D + '/fam3', exist_ok=True)
names = []
for k, v in sorted(out.items()):
    open('%s/fam3/%s.c' % (D, k), 'w', newline='\n').write(v)
    names.append('tmp/f8b488s4/fam3/%s.c' % k)
open(D + '/fam3/list.txt', 'w', newline='\n').write('\n'.join(names) + '\n')
print(len(names), 'variants')
