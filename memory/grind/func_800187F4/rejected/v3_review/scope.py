import json,re,difflib
M=json.load(open('tmp/ledger_map.json'))
for name in ('decisions','borderline'):
    key='docs/grind/%s.md'%name
    H=open('tmp/rv3/%s.head.md'%name,encoding='utf-8',errors='surrogateescape').read().split('\n')
    N=open('tmp/rv3/%s.new.md'%name,encoding='utf-8',errors='surrogateescape').read().split('\n')
    # independent: difflib opcodes
    sm=difflib.SequenceMatcher(None,H,N,autojunk=False)
    dels=[];reps=[]
    for t,i1,i2,j1,j2 in sm.get_opcodes():
        if t=='delete': dels.append((i1+1,i2))
        elif t=='replace': reps.append((i1+1,i2,j1+1,j2))
        elif t=='insert': print('INSERT',name,j1,j2)
    gone=set(M[key]['gone'])
    mine=set()
    for a,b in dels: mine.update(range(a,b+1))
    print(name,'difflib delete ranges',len(dels),'lines',len(mine),'map gone',len(gone),'replace ops',len(reps))
    # headings
    heads=[i+1 for i,l in enumerate(H) if l.startswith('## ')]
    # group gone into contiguous ranges
    g=sorted(gone); rng=[]
    for x in g:
        if rng and x==rng[-1][1]+1: rng[-1][1]=x
        else: rng.append([x,x])
    print(' gone ranges',len(rng))
    fence=False; infence=set()
    for i,l in enumerate(H,1):
        if l.startswith('```'): fence=not fence
        if fence: infence.add(i)
    for a,b in rng:
        hs=[h for h in heads if a<=h<=b]
        ok = H[a-1].startswith('## ') and (b+1 in heads or b==len(H))
        nxt=H[b] if b<len(H) else '<EOF>'
        flag=''
        if not ok: flag+=' NOT-WHOLE'
        if 'DISCARDED' in nxt: flag+=' NEXT-IS-DISCARD-MARKER'
        if any(x in infence for x in range(a,b+1)): flag+=' FENCE'
        if name=='decisions':
            bad=[h for h in hs if 'OWNER RULING' not in H[h-1]]
            if bad: flag+=' NONRULING-HEAD:'+str(bad)
        print(' %d-%d (%d heads)%s | %s'%(a,b,len(hs),flag,H[a-1][:110]))
        if len(hs)>1:
            for h in hs[1:]: print('     +',h,H[h-1][:110])
