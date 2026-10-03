import re,sys
rows=[]
for l in open('tmp/pad-survey/nm.txt'):
    p=l.split()
    if len(p)==4: a,s,t,n=p; rows.append((int(a,16),int(s,16),t,n))
    elif len(p)==3: a,t,n=p; rows.append((int(a,16),None,t,n))
rows=[r for r in rows if 0x800164f8<=r[0]<0x8008d070 and r[2] in 'TtA' and not r[3].endswith('NON_MATCHING')]
# dedupe by address keep first with size
by={}
for r in rows:
    if r[0] not in by or (by[r[0]][1] is None and r[1] is not None): by[r[0]]=r
rs=sorted(by.values())
for i,r in enumerate(rs[:-1]):
    a,s,t,n=r; nx=rs[i+1][0]
    if s is None: continue
    end=a+s
    if end<nx:
        print(f"{n:32s} {a:08x} size={s:#x} end={end:08x} next={rs[i+1][3]:28s} {nx:08x} gap={nx-end} nextmod16={nx%16:x} mod8={nx%8}")
