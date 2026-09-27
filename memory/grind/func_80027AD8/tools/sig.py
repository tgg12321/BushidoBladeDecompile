import sys,re
s=open(sys.argv[1]).read()
m=re.search(r'\nfunc_80027AD8:\n(.*?)\.end\tfunc_80027AD8',s,re.S)
body=[l.strip().replace('\t',' ') for l in m.group(1).split('\n')]
fr=[l for l in body if l.startswith('.frame')][0].split('#')[0]
opp=[l for l in body[:40] if re.match(r'lw \$\d+,0\(\$17\)',l)]
rec=[l for l in body[:60] if re.match(r'lw \$\d+,(92|100)\(\$sp\)',l)]
fp=[l for l in body if l.startswith('move $fp')]
a3=[l for l in body[:40] if l.startswith('sw $7,')]
print(fr,'| opp:',opp,'| rec:',rec,'| fp:',fp[:2],'| a3spill:',a3)
