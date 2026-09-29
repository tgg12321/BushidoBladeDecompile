import itertools
V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
b=open(V+'r11pv_nforce.c').read()
ind='        '
b=b.replace(ind+'nforce_add = node[7];\n', ind+'do { nforce_add = node[7]; } while (0); /* FAKE */\n')
cands=['vx = node[3];','vy = node[4];','vz = node[5];','bits = node[9];','nforce_sub = node[8];','bits2 = node[11];',
 'node[3] = (SCR->vel[0] * 7) >> 3;','node[0] = SCR->pos[0] + SCR->dpos[0] + node[3];','node[4] = ((SCR->vel[1] * 7) >> 3) + 0x190;',
 'node[1] = SCR->pos[1] + SCR->dpos[1] + node[4];','node[5] = (SCR->vel[2] * 7) >> 3;','node[2] = SCR->pos[2] + SCR->dpos[2] + node[5];']
for c in cands: assert (ind+c+'\n') in b, c
tags=[]
for k in range(0,4):
  for sub in itertools.combinations(range(len(cands)),k):
    s=b
    for i in sub:
        s=s.replace(ind+cands[i]+'\n', ind+'do { '+cands[i]+' } while (0); /* FAKE */\n')
    t='nf3_'+('_'.join(map(str,sub)) or 'x')
    open(O+t+'.c','w',newline='\n').write(s); tags.append(t)
open('tmp/rv187/tags_f.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
