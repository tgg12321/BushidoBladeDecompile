import re,subprocess,json,difflib
def sh(*a): return subprocess.run(a,capture_output=True).stdout.decode('utf-8','surrogateescape')
DEC,BL='docs/grind/decisions.md','docs/grind/borderline.md'
A=json.load(open('tmp/ledger_map.json'))
MP={p:{int(k):v for k,v in A[p]['map'].items()} for p in A}; G={p:set(A[p]['gone']) for p in A}
NL={DEC:30385,BL:1002}
changed=set(sh('git','diff','--name-only').split())
files=[f for f in sh('git','ls-files').split('\n') if f and not f.startswith(('disc/','asm/','metrics/')) and not f.endswith(('.bin','.BIN','.png','.gz','.o','.exe','.EXE','.jsonl','.pdf'))]
fname=re.compile(r"[\w./-]+\.(?:md|c|h|s|py|ps1|sh|txt|json|csv|ld|yaml|jsonl|S|inc|patch|log)\b|this file|this ledger|the ledger")
num=re.compile(r"(?<![\w.#x@-])(\d{2,5})(?![\w.]?\d)(?!-\d)")
hits=[]
for f in files:
    try: o=sh('git','show','HEAD:'+f)
    except Exception: continue
    if not re.search(r'decisions|borderline|this file',o): continue
    n=open(f,'rb').read().decode('utf-8','surrogateescape') if f in changed else o
    # set of old positions of numbers that changed: approximate via per-line token diff
    ol=o.split('\n')
    if f in (DEC,BL): pass
    nl=n.split('\n')
    if f in (DEC,BL):
        olk=[(i,l) for i,l in enumerate(ol,1) if i not in G[f]]
    else: olk=list(enumerate(ol,1))
    assert len(olk)==len(nl),(f,len(olk),len(nl))
    for (oi,lo),ln in zip(olk,nl):
        for m in num.finditer(lo):
            v=int(m.group(1))
            # context: preceding 300 chars on this line + previous lines within paragraph
            start=max(0,m.start()-250)
            pre=lo[start:m.start()]
            k=oi-1
            while len(pre)<250 and k>=1 and ol[k-1].strip():
                pre=ol[k-1]+' '+pre; k-=1
            fm=list(fname.finditer(pre))
            near=fm[-1].group(0) if fm else None
            if near and 'decisions.md' in near: led=DEC
            elif near and 'borderline.md' in near: led=BL
            elif (near in ('this file','this ledger','the ledger') or near is None) and f in (DEC,BL): led=f
            else: continue
            if v<2 or v>=NL[led]: continue
            if not (v in G[led] or MP[led].get(v)!=v): continue
            # was this number changed? check if the new line has the same number at same relative token spot
            if lo==ln: stat='UNCHANGED-LINE'
            else:
                # if the exact substring (with 3 chars context) survives unchanged in new line, likely unchanged
                ctx=lo[max(0,m.start()-3):m.end()+2]
                stat='CTX-SURVIVES' if ctx in ln and '@f2bf53757' not in ln[max(0,ln.find(ctx)-30):ln.find(ctx)+len(ctx)] else 'changed'
            if stat=='changed': continue
            between=pre[fm[-1].end():] if fm else pre
            hits.append((f,oi,v,led[11:],stat,(pre[-80:]+'<<'+m.group(1)+'>>'+lo[m.end():m.end()+30]).replace('\n',' ')))
print(len(hits))
for h in hits: print(h)
