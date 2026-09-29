import sys, itertools, re
D='tmp/func_800187F4/'
tags=[]
def w(tag,s):
    open(D+tag+'.c','w',newline='\n').write(s); tags.append(tag)
ANCH={
 'top':'        SCR->pos[0] = node[0];\n',
 'ff':'        if (node[6] >= -0xFF) {\n',
 'vx':'        vx = node[3];\n',
 'nfa':'        bits = node[9];\n',          # after bits load (just before loop 1 header) -> insert before header instead
}
def early(s, var, lim, pos):
    hdr=f'for ({var} = 0; {var} < {lim}; {var}++) {{\n'
    assert s.count(hdr)==1,(var,lim)
    s=s.replace(hdr, f'for (; {var} < {lim}; {var}++) {{\n')
    a=pos
    assert s.count(a)>=1,(pos,)
    return s.replace(a, a.replace(a.lstrip(), f'{var} = 0;\n') + a if False else (re.match(r' *',a).group(0)+f'{var} = 0;\n'+a),1)
bases={'rv3x_4000000':('idx_add','idx_sub','idx_sph'),'rv3x_1000000':('idx_add','idx','idx'),'rv3x_2000000':('idx','idx_sub','idx'),
       'rv3x_4100000':('idx_add','idx_sub','idx_sph'),'rv3x_3000000':('idx','idx','idx_sph')}
for b,(a_,s_,e_) in bases.items():
    s0=open(D+b+'.c').read()
    na='nforce_add' if 'nforce_add' in s0 else 'nforce'
    ns='nforce_sub' if 'nforce_sub' in s0 else 'nforce'
    subpos={'loop':None,'bits2':f'        {ns} = node[8];\n','preadd':f'        for (','vx':ANCH['vx'],'ff':ANCH['ff'],'top':ANCH['top']}
    addpos={'loop':None,'vx':ANCH['vx'],'ff':ANCH['ff'],'top':ANCH['top']}
    for ap,sp in itertools.product(addpos,subpos):
        if a_=='idx' and ap!='loop': continue
        if s_=='idx' and sp!='loop': continue
        if (ap,sp)==('loop','loop'): continue
        s=s0
        if sp!='loop':
            pos=subpos[sp]
            if sp=='preadd':
                pos=f'        for ({a_} = 0; {a_} < {na}; {a_}++) {{\n'
            s=early(s,s_,ns,pos)
        if ap!='loop':
            s=early(s,a_,na,addpos[ap])
        w(f'rv3I_{b[5:]}_{ap}_{sp}',s)
    if e_!='idx':
        for ep,pos in {'cpos':'            SCR->cpos[0] = SCR->pos[0] >> 5;\n','coll':'            delta = SCR->pos[1] - SCR->ground;\n'}.items():
            s=s0
            if pos not in s: pos=pos.replace('delta','dg')
            s=early(s,e_,'SCR->nsph',pos)
            w(f'rv3I_{b[5:]}_e{ep}',s)
open('tmp/rv3/init_tags.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
