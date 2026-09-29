import itertools, os, sys
os.environ['R11BASE']='rv3_c7'
sys.path.insert(0,'memory/grind/func_800187F4/r11/tools')
import mkpv3 as M
D='tmp/func_800187F4/'
base=open(D+'rv3_c7.c').read()
def parts3(var, fresh):
    # set partitions of 3 values: list of name-lists
    a,b,c=fresh
    return [[var,var,var],[a,var,var],[var,b,var],[var,var,c],[a,b,c]]
opts={
 'idx': parts3('idx',["idx_add","idx_sub","idx_sph"]),
 'nforce': [['nforce','nforce'],["nforce_add","nforce_sub"]],
 'temp': parts3('temp',["lzc_in","lut1","lut2"]),
 'work': [['work','work'],["sq1","dist1"]],
 'delta': [['delta','delta'],["dg","dy0"]],
 'nbits': [['nbits','nbits'],["lzcount","shift"]],
 'nbits2': [['nbits2','nbits2'],["lzcount2","shift2"]],
}
order=['idx','nforce','temp','work','delta','nbits','nbits2']
tags=[]
for combo in itertools.product(*[range(len(opts[v])) for v in order]):
    if sum(combo)==0: continue
    s=base
    for v,k in zip(order,combo):
        if k: s=M.VARS[v][0](s, opts[v][k])
    tag='rv3x_'+''.join(str(k) for k in combo)
    open(D+tag+'.c','w',newline='\n').write(s)
    tags.append(tag)
open('tmp/rv3/combo_tags.txt','w',newline='\n').write('\n'.join(tags)+'\n')
print(len(tags))
