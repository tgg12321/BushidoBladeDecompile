#!/usr/bin/env python3
"""Ruling 11 (D)(3) sanctioned-family escapes on the one-variable-per-value twin
(split.c): FAKE self-assignments, dead stores, combine-foldable `+ v - v`
chain-extenders, a pointer alias, the if/else (duplicated-into-arms) clamp and a
do{}while(0) wrapper, placed on the split SR value (sr_rate) and SL value
(sl_rate).  Each variant is split.c plus the named construct only (plus a
declaration move to function scope where the construct sits outside the value's
block).  usage (WSL, repo root): python3 tmp/f8b488s4/mkfam.py -> tmp/f8b488s4/fam/*.c"""
import os
D = os.path.dirname(os.path.abspath(__file__))
base = open(D + '/split.c').read()
F = ' /* FAKE */'


def one(s, o, n):
    assert s.count(o) == 1, (o, s.count(o))
    return s.replace(o, n)


def fscope(s, v):
    """move `u16 v;` from its block to function scope"""
    s = one(s, '            u16 %s;\n' % v, '')
    return one(s, '    s32 bSetAll;\n', '    s32 bSetAll;\n    u16 %s;\n' % v)


SR_CLAMP = '            if (sr_rate >= 0x80) {\n                sr_rate = 0x7F;\n            }\n'
SR_LOAD = '            sr_rate = attr->sr;\n'
SR_SMODE = '            smode = 0x100;\n'
SR_AND = '            adsr &= 0x3F;\n'
SR_IOR = '            *(volatile u16 *)(_spu_RXX + (pos + 5) * 2) = adsr | ((sr_rate | smode) << 6);\n'
SL_LOAD = '            sl_rate = attr->sl;\n'
SL_CLAMP = '            if (sl_rate >= 0x10) {\n                sl_rate = 0xF;\n            }\n'
SL_IOR = '            *(volatile u16 *)(_spu_RXX + (pos + 4) * 2) = (adsr & 0xFFF0) | sl_rate;\n'
RR_AND = '            adsr &= 0xFFC0;\n'
DR_AND = '            adsr &= 0xFF0F;\n'
TAIL = '    v = 1;\n'
out = {}
# ---- SR value ----
out['sr_self_clamp'] = one(base, SR_CLAMP, SR_CLAMP + '            sr_rate = sr_rate;' + F + '\n')
out['sr_self_ior'] = one(base, SR_IOR, '            sr_rate = sr_rate;' + F + '\n' + SR_IOR)
out['sr_dead_top'] = one(base, SR_LOAD, '            sr_rate = 0;' + F + '\n' + SR_LOAD)
out['sr_chain_ior'] = one(base, SR_IOR, SR_IOR.replace('((sr_rate | smode) << 6)', '(((sr_rate | smode) + sr_rate - sr_rate) << 6)').rstrip('\n') + F + '\n')
out['sr_chain_and'] = one(base, SR_AND, '            adsr &= 0x3F + sr_rate - sr_rate;' + F + '\n')
out['sr_chain_smode'] = one(base, SR_SMODE, '            smode = 0x100 + sr_rate - sr_rate;' + F + '\n')
s = one(base, SR_AND, '            adsr &= 0x3F + sr_rate - sr_rate;' + F + '\n')
out['sr_chain_and_ior'] = one(s, SR_IOR, SR_IOR.replace('((sr_rate | smode) << 6)', '(((sr_rate | smode) + sr_rate - sr_rate) << 6)').rstrip('\n') + F + '\n')
s = one(base, SR_SMODE, '            smode = 0x100 + sr_rate - sr_rate;' + F + '\n')
s = one(s, SR_AND, '            adsr &= 0x3F + sr_rate - sr_rate;' + F + '\n')
out['sr_chain_x3'] = one(s, SR_IOR, SR_IOR.replace('((sr_rate | smode) << 6)', '(((sr_rate | smode) + sr_rate - sr_rate) << 6)').rstrip('\n') + F + '\n')
s = one(base, '            u16 sr_rate;\n', '            u16 sr_rate;\n            u16 *sr_p;\n')
out['sr_alias'] = one(s, SR_IOR, '            sr_p = &sr_rate;' + F + '\n' + SR_IOR.replace('(sr_rate | smode)', '(*sr_p | smode)'))
out['sr_ifelse_arms'] = one(base, SR_LOAD + SR_CLAMP,
                            '            if (attr->sr >= 0x80) {\n                sr_rate = 0x7F;\n            } else {\n                sr_rate = attr->sr;\n            }\n')
out['sr_dowhile0'] = one(base, SR_LOAD + SR_CLAMP, '            do {\n' + SR_LOAD + SR_CLAMP + '            } while (0);\n')
# ---- SL value ----
out['sl_self_clamp'] = one(base, SL_CLAMP, SL_CLAMP + '            sl_rate = sl_rate;' + F + '\n')
out['sl_dead_top'] = one(base, SL_LOAD, '            sl_rate = 0;' + F + '\n' + SL_LOAD)
out['sl_chain_ior'] = one(base, SL_IOR, SL_IOR.replace('| sl_rate;', '| (sl_rate + sl_rate - sl_rate);' + F))
s = fscope(base, 'sl_rate')
out['sl_dead_in_sr'] = one(s, SR_AND, SR_AND + '            sl_rate = 0;' + F + '\n')
out['sl_chain_in_sr'] = one(s, SR_AND, '            adsr &= 0x3F + sl_rate - sl_rate;' + F + '\n')
out['sl_chain_in_rr'] = one(s, RR_AND, '            adsr &= 0xFFC0 + sl_rate - sl_rate;' + F + '\n')
out['sl_chain_in_dr'] = one(s, DR_AND, '            adsr &= 0xFF0F + sl_rate - sl_rate;' + F + '\n')
out['sl_chain_after_loop'] = one(s, TAIL, '    v = 1 + sl_rate - sl_rate;' + F + '\n')
out['sl_self_in_sr'] = one(s, SR_AND, SR_AND + '            sl_rate = sl_rate;' + F + '\n')
out['sl_ifelse_arms'] = one(base, SL_LOAD + SL_CLAMP,
                            '            if (attr->sl >= 0x10) {\n                sl_rate = 0xF;\n            } else {\n                sl_rate = attr->sl;\n            }\n')
s = one(base, '            u16 sl_rate;\n', '            u16 sl_rate;\n            u16 *sl_p;\n')
out['sl_alias'] = one(s, SL_IOR, '            sl_p = &sl_rate;' + F + '\n' + SL_IOR.replace('| sl_rate;', '| *sl_p;'))
# ---- both together: the strongest SR escape + each SL escape that leaves the SL block ----
for k in ('sl_chain_in_sr', 'sl_chain_in_rr', 'sl_chain_in_dr', 'sl_chain_after_loop'):
    s = out[k]
    s = one(s, SR_AND if k != 'sl_chain_in_sr' else '            adsr &= 0x3F + sl_rate - sl_rate;' + F + '\n',
            ('            adsr &= 0x3F + sr_rate - sr_rate;' + F + '\n') if k != 'sl_chain_in_sr'
            else '            adsr &= 0x3F + sl_rate - sl_rate + sr_rate - sr_rate;' + F + '\n')
    out['both_' + k[3:]] = s
os.makedirs(D + '/fam', exist_ok=True)
names = []
for k, v in sorted(out.items()):
    fn = '%s/fam/%s.c' % (D, k)
    open(fn, 'w', newline='\n').write(v)
    names.append('tmp/f8b488s4/fam/%s.c' % k)
open(D + '/fam/list.txt', 'w', newline='\n').write('\n'.join(names) + '\n')
print(len(names), 'variants')
