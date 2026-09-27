import re, sys
# usage: fx.py <dumpfile> <regex> [ctx]   -- print matching RTL insns (whole insn) within func_8001A820
path, pat = sys.argv[1], sys.argv[2]
ctx = int(sys.argv[3]) if len(sys.argv) > 3 else 0
t = open(path).read()
s = t.find(';; Function func_8001A820')
e = t.find(';; Function', s + 10)
t = t[s:e if e > 0 else None]
insns = re.split(r"\n\n", t)
for i, ins in enumerate(insns):
    if re.search(pat, ins):
        for j in range(max(0, i - ctx), min(len(insns), i + ctx + 1)):
            print(insns[j])
        print('-----')
