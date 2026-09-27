"""Show the semantic-ish diff of permuter outputs vs base.c (normalizes parens/whitespace).
usage: python3 permdiff.py <workspace> [maxscore]"""
import sys, os, re, difflib
ws = sys.argv[1]
mx = int(sys.argv[2]) if len(sys.argv) > 2 else 10**9
def norm(src):
    src = re.sub(r'/\*.*?\*/', '', src, flags=re.S)
    body = src[src.index('func_80027AD8(s32 pass'):]
    toks = re.findall(r'[A-Za-z_]\w*|0x[0-9A-Fa-f]+|\d+|->|\+\+|--|[<>=!]=|&&|\|\||<<|>>|[^\s()]', body)
    # split into statements at ; { }
    out, cur = [], []
    for t in toks:
        if t in ('{', '}'):
            continue
        cur.append(t)
        if t == ';':
            out.append(' '.join(cur)); cur = []
    return out
base = norm(open(os.path.join(ws, 'base.c')).read())
dirs = sorted((d for d in os.listdir(ws) if d.startswith('output-')), key=lambda d: int(d.split('-')[1]))
for d in dirs:
    sc = int(d.split('-')[1])
    if sc > mx:
        continue
    o = norm(open(os.path.join(ws, d, 'source.c')).read())
    diff = [l for l in difflib.unified_diff(base, o, lineterm='', n=0) if l[:1] in '+-' and not l.startswith(('+++', '---'))]
    print(f'== {d}')
    for l in diff[:12]:
        print('  ' + l[:160])
