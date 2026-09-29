import re,glob,os,collections
funcs=[]
for f in glob.glob('asm/funcs/*.s'):
    first=None
    lines=open(f,encoding='utf-8',errors='replace').read().splitlines()
    for l in lines:
        m=re.match(r'\s*/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s',l)
        if m: first=int(m.group(1),16); break
    if first: funcs.append((first,os.path.basename(f)[:-2],lines))
funcs.sort()
lo,hi=0x8006E500,0x80073300
acc=collections.defaultdict(dict)
for a,n,lines in funcs:
    if not lo<=a<hi: continue
    forms=collections.defaultdict(set)
    for l in lines:
        m=re.search(r'\*/\s+(\w+)\s+(.*)',l)
        if not m: continue
        op,args=m.groups()
        for s in re.findall(r'%(gp_rel|hi|lo)\((D_\w+|g_\w+)\)',args):
            kind,sym=s
            forms[sym].add(f"{op}:{kind}")
    acc[(a,n)]=forms
syms=collections.defaultdict(list)
for (a,n),forms in sorted(acc.items()):
    for s,fs in forms.items(): syms[s].append((a,n,sorted(fs)))
for s,l in sorted(syms.items()):
    kinds=set(k for _,_,fs in l for k in fs)
    if len(l)>1:
        print(s)
        for a,n,fs in l: print(f"   {a:08X} {n}: {' '.join(fs)}")
