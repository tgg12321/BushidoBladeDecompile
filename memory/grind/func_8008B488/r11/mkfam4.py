#!/usr/bin/env python3
"""Fourth family round (layer-2 round-2 objection): a FAKE write of the SL variable and
its chain-extender read in DIFFERENT basic blocks of the same iteration, so combine
cannot fold the write into the read.  Writes are loads/computations (x_*) or register
copies of another block's value (c_*).  sl_rate is declared in the loop body.  Each is
split.c plus the named construct only, alone ("_n") and with the SR chain-extender of
fam/sr_chain_ior.c ("_c").  usage (WSL, repo root): python3 tmp/f8b488s4/mkfam4.py"""
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
SR_CLAMP_END = '            if (sr_rate >= 0x80) {\n                sr_rate = 0x7F;\n            }\n'
SR_SMODE = '            smode = 0x100;\n'
SR_AND = '            adsr &= 0x3F;\n'
SR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((sr_rate | smode) << 6);'
SR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode)' + X + ') << 6);' + F
SR_ST_C = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode) + sr_rate - sr_rate) << 6);' + F
SR_ST_XC = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (((sr_rate | smode) + sr_rate - sr_rate' + X + ') << 6);' + F
RR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | (rr_rate | rmode);'
RR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((rr_rate | rmode)' + X + ');' + F
DR_ST = '*(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | (dr_rate << 4);'
DR_ST_X = '*(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = adsr | ((dr_rate' + X + ') << 4);' + F
AR_CLAMP_END = '            if (ar_rate >= 0x80) {\n                ar_rate = 0x7F;\n            }\n'
RR_CLAMP_END = '            if (rr_rate >= 0x20) {\n                rr_rate = 0x1F;\n            }\n'
out = {}


def pair(tag, w, rd_old, rd_new, sr_ok=True):
    out[tag + '_n'] = one(w, rd_old, rd_new)
    if rd_old == SR_ST:
        out[tag + '_c'] = one(w, SR_ST, SR_ST_XC)
    else:
        out[tag + '_c'] = one(one(w, rd_old, rd_new), SR_ST, SR_ST_C)


# write after the SR clamp (before the smode switch), read in the SR store: the switch's blocks lie between
for t, src in (('x_load', 'attr->sl'), ('x_pos', 'pos'), ('c_srrate', 'sr_rate')):
    pair('sr_early_' + t, one(s0, SR_CLAMP_END, SR_CLAMP_END + '            sl_rate = %s;' % src + F + '\n'), SR_ST, SR_ST_X)
# write in the SR block after the mask, read in the RR store (the RR guard and clamp lie between)
for t, src in (('x_load', 'attr->sl'), ('x_adsr', 'adsr'), ('c_srrate', 'sr_rate')):
    pair('sr_to_rr_' + t, one(s0, SR_AND, SR_AND + '            sl_rate = %s;' % src + F + '\n'), RR_ST, RR_ST_X)
# write after the AR clamp, read in the DR store
for t, src in (('x_load', 'attr->sl'), ('c_arrate', 'ar_rate')):
    pair('ar_to_dr_' + t, one(s0, AR_CLAMP_END, AR_CLAMP_END + '            sl_rate = %s;' % src + F + '\n'), DR_ST, DR_ST_X)
# write after the RR clamp (before the rmode switch), read in the RR store
for t, src in (('x_load', 'attr->sl'), ('c_rrrate', 'rr_rate')):
    pair('rr_early_' + t, one(s0, RR_CLAMP_END, RR_CLAMP_END + '            sl_rate = %s;' % src + F + '\n'), RR_ST, RR_ST_X)
os.makedirs(D + '/fam4', exist_ok=True)
names = []
for k, v in sorted(out.items()):
    open('%s/fam4/%s.c' % (D, k), 'w', newline='\n').write(v)
    names.append('tmp/f8b488s4/fam4/%s.c' % k)
open(D + '/fam4/list.txt', 'w', newline='\n').write('\n'.join(names) + '\n')
print(len(names), 'variants')
