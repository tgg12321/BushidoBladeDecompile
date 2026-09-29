import sys, itertools
sys.path.insert(0,'tmp/rv3')
import gotoloop as G
D='tmp/func_800187F4/'
bases=['rv3_c7','rv3x_0100000','rv3x_4000000','rv3x_1000000','rv3x_2000000','rv3x_3000000','rv3x_4100000','rv3x_0001000','rv3x_0000100','rv3x_0040000','rv3x_0000010']
F01=['for','guard','jtest','dowhile','while']
F2=['for','guard','jtest']
tags=[]
for b in bases:
    s0=open(D+b+'.c').read()
    for f0,f1,f2 in itertools.product(F01,F01,F2):
        if (f0,f1,f2)==('for','for','for'): continue
        s=s0
        for k,f in ((2,f2),(1,f1),(0,f0)):
            if f!='for': s=G.to_goto(s,k,f)
        tag=f'rv3L_{b[4:]}_{f0[0]}{f1[0]}{f2[0]}'.replace('rv3L_c7','rv3L_c7')
        open(D+tag+'.c','w',newline='\n').write(s); tags.append(tag)
open('tmp/rv3/loop_tags.txt','w',newline='\n').write('\n'.join(tags)+'\n')
print(len(tags))
