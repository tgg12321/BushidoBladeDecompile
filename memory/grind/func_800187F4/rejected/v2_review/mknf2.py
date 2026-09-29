V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
b=open(V+'r11pv_nforce.c').read()
ind='        '
S=['vx = node[3];','vy = node[4];','vz = node[5];','nforce_add = node[7];','bits = node[9];']
head=''.join(ind+x+'\n' for x in S)
assert head in b
tags=[]
for a in range(0,4):
  for e in range(3,5):
    for W in (1,2):
      inner=' '.join(S[a:e+1])
      for _ in range(W): inner='do { '+inner+' } while (0);'
      L=S[:a]+[inner+' /* FAKE */']+S[e+1:]
      h=''.join(ind+x+'\n' for x in L)
      t='nf2_a%de%dW%d'%(a,e,W)
      open(O+t+'.c','w',newline='\n').write(b.replace(head,h)); tags.append(t)
# also: wrap only the loop test? wrap the for loop of add
open('tmp/rv187/tags_e.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
