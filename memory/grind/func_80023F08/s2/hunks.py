import sys,re
txt=open(sys.argv[1],encoding='utf-8',errors='replace').read()
parts=re.split(r'(?=^@ hunk )',txt,flags=re.M)
lo=int(sys.argv[2]) if len(sys.argv)>2 else 0
hi=int(sys.argv[3]) if len(sys.argv)>3 else 99999
for p in parts:
    m=re.match(r'@ hunk (\d+)/\d+\s+(\w+)\s+target\[(\d+)\] ours\[(\d+)\]\s+—\s+([\w-]+)',p)
    if not m: continue
    n=int(m.group(1))
    if n<lo or n>hi: continue
    if 'not-scored' in m.group(5) : continue
    if len(sys.argv)>4 and sys.argv[4]=='src' and m.group(5)!='source-level': continue
    print(p.rstrip()[:3000])
