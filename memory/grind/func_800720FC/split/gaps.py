import re,glob,os
funcs=[]
for f in glob.glob('asm/funcs/*.s'):
    first=None;n=0;last=None
    for l in open(f,encoding='utf-8',errors='replace'):
        m=re.match(r'\s*/\*\s*[0-9A-F]+\s+([0-9A-F]{8})\s+[0-9A-F]{8}\s*\*/',l)
        if m:
            a=int(m.group(1),16)
            if first is None: first=a
            last=a
    if first: funcs.append((first,last+4,os.path.basename(f)[:-2]))
funcs.sort()
lo,hi=0x80047ED0,0x80077B30
prev=None
for s,e,n in funcs:
    if lo<=s<hi:
        if prev and s!=prev[1]:
            print(f"GAP {prev[2]} end {prev[1]:08X} -> {n} start {s:08X} ({s-prev[1]} bytes)")
        prev=(s,e,n)
