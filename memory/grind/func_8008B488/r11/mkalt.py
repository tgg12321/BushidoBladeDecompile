#!/usr/bin/env python3
"""Alternative closing shapes for the record: SL shares with AR or RR (the other
partners that fix SL's seat), and the SR seat is then supplied by the FAKE
chain-extender of fam/sr_chain_ior.c.  Also type respellings of split.c.
usage (WSL, repo root): python3 tmp/f8b488s4/mkalt.py"""
import os, subprocess
D = os.path.dirname(os.path.abspath(__file__))
F = ' /* FAKE */'
SR_IOR = 'adsr | ((sr_rate | smode) << 6);'
SR_IOR_CH = 'adsr | (((sr_rate | smode) + sr_rate - sr_rate) << 6);' + F
os.makedirs(D + '/alt', exist_ok=True)
names = []
for spec, tag in (('AR,SL', 'arsl'), ('RR,SL', 'rrsl')):
    subprocess.run(['python3', D + '/gen_part.py', D + '/alt/tmp_' + tag, spec], check=True, capture_output=True)
    fn = [f for f in os.listdir(D + '/alt/tmp_' + tag) if f.endswith('.c')][0]
    s = open(D + '/alt/tmp_' + tag + '/' + fn).read()
    for k, v in ((tag + '_plain', s), (tag + '_srchain', s.replace(SR_IOR, SR_IOR_CH))):
        open('%s/alt/%s.c' % (D, k), 'w', newline='\n').write(v)
        names.append('tmp/f8b488s4/alt/%s.c' % k)
split = open(D + '/split.c').read()
for ty in ('s32', 'u32', 's16', 'u8'):
    s = split.replace('            u16 sr_rate;', '            %s sr_rate;' % ty).replace('            u16 sl_rate;', '            %s sl_rate;' % ty)
    open('%s/alt/type_%s.c' % (D, ty), 'w', newline='\n').write(s)
    names.append('tmp/f8b488s4/alt/type_%s.c' % ty)
# function-scope per-value locals (the old candidate.c shape), and loop-body scope
s = split
for v in ('sr_rate', 'sl_rate'):
    s = s.replace('            u16 %s;\n' % v, '', 1)
open(D + '/alt/scope_func.c', 'w', newline='\n').write(s.replace('    s32 bSetAll;\n', '    s32 bSetAll;\n    u16 sr_rate;\n    u16 sl_rate;\n', 1))
open(D + '/alt/scope_loop.c', 'w', newline='\n').write(s.replace('    for (voice = 0; voice < 24; voice++) {\n', '    for (voice = 0; voice < 24; voice++) {\n        u16 sr_rate;\n        u16 sl_rate;\n', 1))
open(D + '/alt/scope_loop_rev.c', 'w', newline='\n').write(s.replace('    for (voice = 0; voice < 24; voice++) {\n', '    for (voice = 0; voice < 24; voice++) {\n        u16 sl_rate;\n        u16 sr_rate;\n', 1))
names += ['tmp/f8b488s4/alt/scope_func.c', 'tmp/f8b488s4/alt/scope_loop.c', 'tmp/f8b488s4/alt/scope_loop_rev.c']
# reuse form with other types for temp (codegen check)
final = open(D + '/final.c').read()
for ty in ('s32', 'u32'):
    open('%s/alt/final_%s.c' % (D, ty), 'w', newline='\n').write(final.replace('        u16 temp;', '        %s temp;' % ty, 1))
    names.append('tmp/f8b488s4/alt/final_%s.c' % ty)
open(D + '/alt/list.txt', 'w', newline='\n').write('\n'.join(names) + '\n')
print(len(names))
