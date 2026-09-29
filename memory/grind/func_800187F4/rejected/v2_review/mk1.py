import os
V='memory/grind/func_800187F4/r11/variants_v2/'
O='tmp/func_800187F4/'
def w(tag, s):
    open(O+tag+'.c','w',newline='\n').write(s)
nf=open(V+'r11pv_nforce.c').read()
w('rv_nf0', nf)
# A1: wrap each load
a1=nf.replace('        nforce_add = node[7];\n','        do { nforce_add = node[7]; } while (0); /* FAKE */\n')
a1=a1.replace('        nforce_sub = node[8];\n','        do { nforce_sub = node[8]; } while (0); /* FAKE */\n')
assert a1!=nf; w('rv_nfA1', a1)
# A1b: only add
a1b=nf.replace('        nforce_add = node[7];\n','        do { nforce_add = node[7]; } while (0); /* FAKE */\n'); w('rv_nfA1b',a1b)
a1c=nf.replace('        nforce_sub = node[8];\n','        do { nforce_sub = node[8]; } while (0); /* FAKE */\n'); w('rv_nfA1c',a1c)
# A2: wrap load+bits load
a2=nf.replace('        nforce_add = node[7];\n        bits = node[9];\n','        do { nforce_add = node[7]; bits = node[9]; } while (0); /* FAKE */\n')
a2=a2.replace('        nforce_sub = node[8];\n        bits2 = node[11];\n','        do { nforce_sub = node[8]; bits2 = node[11]; } while (0); /* FAKE */\n')
assert a2!=nf; w('rv_nfA2',a2)
print('ok')
