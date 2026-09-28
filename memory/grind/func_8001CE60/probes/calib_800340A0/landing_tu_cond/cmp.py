import sys, re, difflib
SYM = [("D_800A3898+1", "D_800A3899"), ("D_800A38AA+1", "D_800A38AB"),("g_sc+1", "D_800A3899"), ("g_sc", "D_800A3898"), ("g_tb+1", "D_800A38AB"), ("g_tb", "D_800A38AA")]
def norm(p):
    t=open(p).read().replace('\r','')
    for a,b in SYM: t=t.replace(a,b)
    out=[];labs={}
    for l in t.splitlines():
        s=l.split('#')[0].strip()
        if not s or s.startswith('.') and not re.match(r'^[.$]L\d+:$',s): continue
        if s.endswith(':') and not re.match(r'^[.$]L\d+:$',s): continue
        for m in re.findall(r'[.$]L\d+',s):
            labs.setdefault(m,'L%d'%len(labs))
        s=re.sub(r'[.$]L\d+',lambda m:labs[m.group(0)],s)
        out.append(re.sub(r'\s+',' ',s))
    return out
a=norm(sys.argv[1]); b=norm(sys.argv[2])
d=[l for l in difflib.unified_diff(a,b,lineterm='',n=0) if not l.startswith(('---','+++','@@'))]
print("differing lines:",len(d), " la count:", sum(1 for x in b if x.startswith('la ')))
for l in d[:40]: print(l)
