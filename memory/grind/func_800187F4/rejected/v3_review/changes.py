import re,subprocess,difflib,json,sys
def sh(*a): return subprocess.run(a,capture_output=True).stdout.decode('utf-8','surrogateescape')
DEC,BL='docs/grind/decisions.md','docs/grind/borderline.md'
H={p:sh('git','show','HEAD:'+p).split('\n') for p in (DEC,BL)}
N={p:open(p,'rb').read().decode('utf-8','surrogateescape').split('\n') for p in (DEC,BL)}
# independent map via difflib equal/replace blocks on whole lines
maps={};gone={}
for p in (DEC,BL):
    sm=difflib.SequenceMatcher(None,H[p],N[p],autojunk=False)
    m={};g=set()
    for t,i1,i2,j1,j2 in sm.get_opcodes():
        if t in('equal',) :
            for k in range(i2-i1): m[i1+k+1]=j1+k+1
        elif t=='replace':
            if i2-i1!=j2-j1: print('UNEQUAL REPLACE',p,i1,i2,j1,j2)
            for k in range(min(i2-i1,j2-j1)): m[i1+k+1]=j1+k+1
            for k in range(j2-j1,i2-i1): g.add(i1+k+1)
        elif t=='delete': g.update(range(i1+1,i2+1))
    maps[p]=m;gone[p]=g
A=json.load(open('tmp/ledger_map.json'))
for p in (DEC,BL):
    am={int(k):v for k,v in A[p]['map'].items()}; ag=set(A[p]['gone'])
    diffs=[k for k in m if am.get(k)!=maps[p].get(k)]
    print(p,'my gone',len(gone[p]),'author gone',len(ag),'map disagreements',len([k for k in maps[p] if am.get(k)!=maps[p][k]]),'gone sym diff',len(gone[p]^ag))
json.dump({p:{'map':maps[p],'gone':sorted(gone[p])} for p in maps},open('tmp/rv3/mymap.json','w'))
