import itertools
V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
b=open(V+'r11pv_nforce.c').read()
ind='        '
b=b.replace(ind+'nforce_add = node[7];\n', ind+'do { nforce_add = node[7]; } while (0); /* FAKE */\n')
cands=[(ind,'vx = node[3];'),(ind,'vy = node[4];'),(ind,'vz = node[5];'),(ind,'bits = node[9];'),(ind,'nforce_sub = node[8];'),(ind,'bits2 = node[11];'),
       (ind+'        ','bits = node[10];'),(ind+'        ','bits2 = node[12];')]
for i,c in cands: assert (i+c+'\n') in b, c
tags=[]
for k in range(4,9):
  for sub in itertools.combinations(range(len(cands)),k):
    s=b
    for j in sub:
        i,c=cands[j]
        s=s.replace(i+c+'\n', i+'do { '+c+' } while (0); /* FAKE */\n')
    t='nf4_'+'_'.join(map(str,sub))
    open(O+t+'.c','w',newline='\n').write(s); tags.append(t)
open('tmp/rv187/tags_g.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
