import sys, itertools, re
sys.path.insert(0,'tmp/rv3')
D='tmp/func_800187F4/'
tags=[]
def w(tag,s):
    open(D+tag+'.c','w',newline='\n').write(s); tags.append(tag)
def early(s, var, lim, pos):
    hdr=f'for ({var} = 0; {var} < {lim}; {var}++) {{\n'
    assert s.count(hdr)>=1
    s=s.replace(hdr, f'for (; {var} < {lim}; {var}++) {{\n')
    assert pos in s,(pos,)
    return s.replace(pos, re.match(r' *',pos).group(0)+f'{var} = 0;\n'+pos,1)
res=open('tmp/rv3/combo_res.txt').read().split('\n')
# all combos with nforce split
bases=[l.split()[0] for l in res if l and l.split()[0][6]=='1']
bases=['rv3_c7']+bases
for b in bases:
    s0=open(D+b+'.c').read()
    na='nforce_add' if 'nforce_add' in s0 else 'nforce'
    ia='idx_add' if 'idx_add' in s0 else 'idx'
    for pn,pos in (('ff','        if (node[6] >= -0xFF) {\n'),):
        w(f'rv3J_{b[4:]}_{pn}', early(s0, ia, na, pos))
open('tmp/rv3/init2_tags.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
