# funcs.py extract|apply <src.c> <dir> [names...]
#   extract: write each named top-level function definition (signature line .. closing '}' at column 0) to <dir>/<name>.c
#   apply:   replace each definition in <src.c> by <dir>/<name>.c (for every <name>.c present, or the named ones)
import os, re, sys

def find(s, name):
    m = re.search(r'^[A-Za-z_][^\n;]*\b%s\(([^;{]*)\)\s*\{\n' % re.escape(name), s, re.M)
    assert m, name
    start = m.start()
    end = s.index('\n}\n', m.end()) + 3
    return start, end

mode, src, d = sys.argv[1], sys.argv[2], sys.argv[3]
names = sys.argv[4:]
s = open(src, encoding='utf-8', newline='').read()
if mode == 'extract':
    os.makedirs(d, exist_ok=True)
    for n in names:
        a, b = find(s, n)
        open(os.path.join(d, n + '.c'), 'w', encoding='utf-8', newline='\n').write(s[a:b])
    print('extracted', len(names))
else:
    if not names:
        names = [f[:-2] for f in sorted(os.listdir(d)) if f.endswith('.c')]
    for n in names:
        a, b = find(s, n)
        t = open(os.path.join(d, n + '.c'), encoding='utf-8').read()
        if not t.endswith('\n'):
            t += '\n'
        s = s[:a] + t + s[b:]
    open(src, 'w', encoding='utf-8', newline='\n').write(s)
    print('applied', len(names))
