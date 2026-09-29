import re, sys
V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
bases=sys.argv[1].split(',')
tags=[]
def blockend(L,i):
    # L[i] ends with '{'; find matching close line index (the line with closing brace at same indent)
    ind=len(L[i])-len(L[i].lstrip())
    for j in range(i+1,len(L)):
        s=L[j]
        if len(s)-len(s.lstrip())==ind and s.strip().startswith('}'):
            return j
    return None
for b in bases:
    src=open(V+b+'.c').read()
    L=src.split('\n')
    start=[k for k,l in enumerate(L) if l.strip().startswith('for (i = 0; i < count')][0]
    n=0
    for k in range(start+1,len(L)-2):
        s=L[k]; t=s.strip()
        if not t or t.startswith('/*') or t.startswith('*') or '@gte' in t: continue
        if re.match(r'^(s32|u8|s16|u16|u32)\b',t): continue
        cand=None
        if t.endswith(';') and not t.startswith(('continue','return','break','}')) and not t.startswith(('if','for','else')):
            cand=(k,k)
        elif (t.startswith('for (') or t.startswith('if (')) and t.endswith('{'):
            e=blockend(L,k)
            if e is None: continue
            # if-chain: extend over else
            while e is not None and L[e].strip().startswith('} else'):
                e2=blockend(L,e)
                if e2 is None: break
                e=e2
            if L[e].strip()!='}': continue
            cand=(k,e)
        if not cand: continue
        a,e=cand
        blk=chr(10).join(L[a:e+1])
        if not L[a].strip().startswith('for (') and ('continue' in blk or 'break' in blk): continue
        ind=s[:len(s)-len(s.lstrip())]
        M=L[:a]+[ind+'do { /* FAKE */']+L[a:e+1]+[ind+'} while (0);']+L[e+1:]
        tag='dw_%s_%03d'%(b,n); n+=1
        open(O+tag+'.c','w',newline='\n').write('\n'.join(M))
        tags.append(tag)
open('tmp/rv187/tags_'+sys.argv[2]+'.txt','w',newline='\n').write('\n'.join(tags)+'\n')
print(len(tags))
