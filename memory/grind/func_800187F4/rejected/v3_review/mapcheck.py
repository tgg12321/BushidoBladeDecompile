import re,subprocess,json
def sh(*a): return subprocess.run(a,capture_output=True).stdout.decode('utf-8','surrogateescape')
DEC,BL='docs/grind/decisions.md','docs/grind/borderline.md'
A=json.load(open('tmp/ledger_map.json'))
for p in (DEC,BL):
    H=sh('git','show','HEAD:'+p).split('\n'); N=open(p,'rb').read().decode('utf-8','surrogateescape').split('\n')
    g=set(A[p]['gone']); am={int(k):v for k,v in A[p]['map'].items()}
    kept=[i for i in range(1,len(H)+1) if i not in g]
    print(p,len(H),len(kept),len(N))
    assert [am[i] for i in kept]==list(range(1,len(N)+1))
    nd=0
    for i in kept:
        a,b=H[i-1],N[am[i]-1]
        if a!=b:
            nd+=1
            if re.sub(r'\d+|@f2bf53757','#',a)!=re.sub(r'\d+|@f2bf53757','#',b):
                print('  NONDIGIT DIFF',i,'->',am[i]); print('   -',a[:300]); print('   +',b[:300])
    print(' lines with differences',nd)
