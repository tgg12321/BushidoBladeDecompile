exec(open('tmp/rev_split/widths.py').read().split('mism=collections')[0])
cnt=collections.defaultdict(collections.Counter)
for f in sorted(files):
    for n,l in enumerate(open(f),1):
        m=ld.search(l)
        if not m or m.group(4) in ('sp','gp'): continue
        off=int(m.group(3),0)
        if off in (0x86,0x26C,0x272,0x364,0x366,0x398,0x39A,0x3F0,0x428,0x438,0x6A):
            cnt[off][m.group(1)]+=1
for k in sorted(cnt): print(hex(k), decl[k], dict(cnt[k]))
