import sys,re
txt=open(sys.argv[1],encoding='utf-8',errors='replace').read()
blocks=re.split(r'\n(?=@ hunk)',txt)
print(blocks[0].split('\n')[2])
for b in blocks[1:]:
    if 'not-scored' in b.split('\n')[0]: continue
    print(b.rstrip())
