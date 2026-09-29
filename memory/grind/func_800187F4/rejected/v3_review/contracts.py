import re
rd=lambda p: open(p,encoding='utf-8',errors='surrogateescape').read()
R=re.compile(r"(\w[\w$]*)\s*(?:\([^)]*\))?\s*=\s*(\w[\w$]*)")
def closure(txt):
    pairs=set(R.findall(txt)); return pairs
H=closure(rd('tmp/rv3/borderline.head.md')); N=closure(rd('tmp/rv3/borderline.new.md'))
lost=H-N
print('alias pairs head',len(H),'new',len(N),'lost',len(lost),'gained',sorted(N-H))
for p in sorted(lost): print('  LOST',p)
# per-name alias sets
names=set(x for p in H|N for x in p)
def al(f,pairs):
    s={f}
    for a,b in pairs:
        if f in (a,b): s.update((a,b))
    return s
diff=[(f,sorted(al(f,H)-al(f,N))) for f in sorted(names) if al(f,H)-al(f,N)]
print('names whose alias set shrank:',len(diff))
for d in diff: print('  ',d)
for nm in ('decisions',):
    for ver in ('head','new'):
        t=rd('tmp/rv3/decisions.%s.md'%ver).split('\n')
        L=[l for l in t if re.search('OWNER-ESCALATION|CANONICAL-ASM GRANT PATH',l)]
        fn=set(re.findall(r'\b(?:func_[0-9A-F]{8}|\w+_[0-9A-F]{8}|[A-Za-z_]\w+)\b',' '.join(L)))
        globals()[ver]=(L,fn)
    print('esc lines head',len(head[0]),'new',len(new[0]))
    lostlines=[l for l in head[0] if l not in new[0]]
    print('lost esc lines',len(lostlines))
    for l in lostlines: print('  ',l[:160])
    print('discard markers head',sum('DISCARDED-SESSION MARKER' in l for l in rd('tmp/rv3/decisions.head.md').split('\n')),'new',sum('DISCARDED-SESSION MARKER' in l for l in rd('tmp/rv3/decisions.new.md').split('\n')))
