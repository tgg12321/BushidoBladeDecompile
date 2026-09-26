"""Extract func_800620B8 from the best N permuter outputs of a workspace into candidate files.
usage: python tmp/func_800620B8/perm/extract.py <ws> <N>  -> prints comma list of candidate paths"""
import os
import re
import sys

ws, n = sys.argv[1], int(sys.argv[2])
outs = []
for d in os.listdir(ws):
    m = re.match(r'output-(\d+)-(\d+)$', d)
    if m:
        outs.append((int(m.group(1)), int(m.group(2)), d))
outs.sort()
paths = []
for sc, k, d in outs[:n]:
    t = open(os.path.join(ws, d, 'source.c'), encoding='utf-8').read()
    a = t.index('void func_800620B8(')
    depth = 0
    i = t.index('{', a)
    j = i
    while True:
        if t[j] == '{':
            depth += 1
        elif t[j] == '}':
            depth -= 1
            if depth == 0:
                break
        j += 1
    p = os.path.join(ws, 'cand_%d_%d.c' % (sc, k))
    open(p, 'w', encoding='utf-8', newline='\n').write(t[a:j + 1] + '\n')
    paths.append(p.replace('\\', '/'))
print(','.join(paths))
