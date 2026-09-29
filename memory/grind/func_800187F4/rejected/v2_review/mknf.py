V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
b=open(V+'r11pv_nforce.c').read()
ind='        '
head=ind+'vx = node[3];\n'+ind+'vy = node[4];\n'+ind+'vz = node[5];\n'+ind+'nforce_add = node[7];\n'+ind+'bits = node[9];\n'
assert head in b
sub=ind+'nforce_sub = node[8];\n'+ind+'bits2 = node[11];\n'
assert sub in b
def wrap(stmt,w):
    s=stmt
    for _ in range(w): s='do { '+s+' } while (0);'
    return s+(' /* FAKE */' if w else '')
tags=[]
for P in range(4):
  for W in range(3):
    for S in range(4):
      L=['vx = node[3];','vy = node[4];','vz = node[5];']
      nf=wrap('nforce_add = node[7];',W)
      L.insert(P,nf)
      h=''.join(ind+x+'\n' for x in L)+ind+'bits = node[9];\n'
      if S==0: sb=sub
      elif S==1: sb=ind+wrap('nforce_sub = node[8];',1)+'\n'+ind+'bits2 = node[11];\n'
      elif S==2: sb=ind+'bits2 = node[11];\n'+ind+'nforce_sub = node[8];\n'
      else: sb=ind+'bits2 = node[11];\n'+ind+wrap('nforce_sub = node[8];',1)+'\n'
      s=b.replace(head,h).replace(sub,sb)
      t='nf_P%dW%dS%d'%(P,W,S)
      open(O+t+'.c','w',newline='\n').write(s); tags.append(t)
open('tmp/rv187/tags_d.txt','w',newline='\n').write('\n'.join(tags)+'\n'); print(len(tags))
