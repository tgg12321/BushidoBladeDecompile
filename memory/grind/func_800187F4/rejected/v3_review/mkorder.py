import sys, itertools, re
D='tmp/func_800187F4/'
tags=[]
def w(tag,s):
    open(D+tag+'.c','w',newline='\n').write(s); tags.append(tag)
for b in ['rv3x_0100000','rv3_c7','rv3x_4100000','rv3x_4000000']:
    s0=open(D+b+'.c').read()
    na = 'nforce_add' if 'nforce_add' in s0 else 'nforce'
    ns = 'nforce_sub' if 'nforce_sub' in s0 else 'nforce'
    ia = 'idx_add' if 'idx_add' in s0 else 'idx'
    loads=['        vx = node[3];\n','        vy = node[4];\n','        vz = node[5];\n',f'        {na} = node[7];\n','        bits = node[9];\n']
    blk=''.join(loads)
    assert blk in s0,(b,)
    # permutations of the five loads: keep vx,vy,vz order, move nforce and bits positions
    for perm in itertools.permutations(range(5)):
        if [p for p in perm if p<3]!=[0,1,2]: continue
        if perm==(0,1,2,3,4): continue
        s=s0.replace(blk,''.join(loads[p] for p in perm))
        w(f'rv3O_{b[4:]}_l{"".join(map(str,perm))}',s)
    # sub loads order swap
    sb=f'        {ns} = node[8];\n        bits2 = node[11];\n'
    s=s0.replace(sb, f'        bits2 = node[11];\n        {ns} = node[8];\n'); assert s!=s0
    w(f'rv3O_{b[4:]}_sw',s)
    # early idx init
    hdr=f'        for ({ia} = 0; {ia} < {na}; {ia}++) {{\n'
    assert hdr in s0
    s1=s0.replace(hdr, f'        for (; {ia} < {na}; {ia}++) {{\n')
    w(f'rv3O_{b[4:]}_i1', s1.replace('        vx = node[3];\n', f'        {ia} = 0;\n        vx = node[3];\n'))
    w(f'rv3O_{b[4:]}_i2', s1.replace('        SCR->pos[0] = node[0];\n', f'        {ia} = 0;\n        SCR->pos[0] = node[0];\n',1))
    w(f'rv3O_{b[4:]}_i3', s1.replace(f'        {na} = node[7];\n', f'        {na} = node[7];\n        {ia} = 0;\n'))
    # nforce loads moved up to top of node body (before pos stores) -- legal: node[7] not written in between? check
    w(f'rv3O_{b[4:]}_n1', s0.replace(f'        {na} = node[7];\n','').replace('        SCR->pos[0] = node[0];\n', f'        {na} = node[7];\n        SCR->pos[0] = node[0];\n',1))
open('tmp/rv3/order_tags.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
