import re,subprocess,json,difflib,collections
def sh(*a): return subprocess.run(a,capture_output=True).stdout.decode('utf-8','surrogateescape')
DEC,BL='docs/grind/decisions.md','docs/grind/borderline.md'
A=json.load(open('tmp/ledger_map.json'))
MP={p:{int(k):v for k,v in A[p]['map'].items()} for p in A}; G={p:set(A[p]['gone']) for p in A}
PH={p:sh('git','show','f2bf53757:'+p).split('\n') for p in (DEC,BL)}
NW={p:open(p,'rb').read().decode('utf-8','surrogateescape').split('\n') for p in (DEC,BL)}
files=[f for f in sh('git','diff','--name-only').split() if f!='metrics/events.jsonl']
tok=re.compile(r"\d+|[A-Za-z_]+|\s+|.",re.S)
fname=re.compile(r"[\w./-]+\.(?:md|c|h|s|py|ps1|sh|txt|json|csv|ld|yaml|jsonl)\b|this file|this ledger")
rows=[]
for f in files:
    o=sh('git','show','HEAD:'+f).split('\n'); n=open(f,'rb').read().decode('utf-8','surrogateescape').split('\n')
    if f in (DEC,BL):
        o=[l for i,l in enumerate(o,1) if i not in G[f]]
    assert len(o)==len(n),f
    for li,(lo,ln) in enumerate(zip(o,n),1):
        if lo==ln: continue
        ot,nt=tok.findall(lo),tok.findall(ln)
        sm=difflib.SequenceMatcher(None,ot,nt,autojunk=False)
        for t,i1,i2,j1,j2 in sm.get_opcodes():
            if t=='equal': continue
            a=''.join(ot[i1:i2]); b=''.join(nt[j1:j2]); pre=''.join(ot[:i1]); post=''.join(ot[i2:])
            rows.append((f,li,t,a,b,pre,post))
print('total change ops',len(rows))
# classify
out=collections.Counter(); bad=[]
for f,li,t,a,b,pre,post in rows:
    if t=='replace' and a.isdigit() and b.isdigit():
        v,w=int(a),int(b)
        # determine ledger by nearest preceding filename within the line (or 'this file')
        fm=list(fname.finditer(pre))
        near=fm[-1].group(0) if fm else None
        if near and ('decisions' in near): led=DEC
        elif near and 'borderline' in near: led=BL
        elif near in ('this file','this ledger') : led=f if f in (DEC,BL) else None
        elif near is None: led='?'
        else: led='OTHER:'+near
        ok = led in (DEC,BL) and MP[led].get(v)==w and v not in G[led]
        dist=len(pre)-fm[-1].end() if fm else -1
        key=('OK' if ok else 'CHECK')
        out[key]+=1
        rows_ctx=(pre[-70:]+'['+a+'->'+b+']'+post[:25]).replace('\n',' ')
        if not ok or dist>40:
            bad.append((key,f,li,led,dist,rows_ctx))
    else:
        out['nonnum:'+t]+=1
        bad.append(('NONNUM',f,li,t,repr(a),repr(b),(pre[-60:]).replace('\n',' ')))
print(out)
for x in bad: print(x)
print('=== OK between-patterns')
pat=collections.Counter(); ex={}
for f,li,t,a,b,pre,post in rows:
    if t=='replace' and a.isdigit() and b.isdigit():
        fm=list(fname.finditer(pre))
        if not fm: continue
        between=pre[fm[-1].start():]
        k=re.sub(r'\d+','N',between)
        k=re.sub(r'[A-Za-z]{4,}','W',k)
        pat[k]+=1; ex.setdefault(k,(f,li,between[-90:]+'['+a+'->'+b+']'+post[:20]))
for k,c in pat.most_common(): print(c,repr(k)[:100],'|',ex[k])
