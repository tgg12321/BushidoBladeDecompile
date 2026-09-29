import difflib, re, sys
def load(p):
    L = []
    for l in open(p):
        l = l.strip()
        if not l:
            continue
        a, _, ins = l.partition(' ')
        L.append((int(a, 16), re.sub(r'\s+', ' ', ins).strip()))
    return L
t = load(sys.argv[1]); o = load(sys.argv[2])
def norm(L):
    b = L[0][0]
    out = []
    for a, ins in L:
        if re.match(r'^(b|j )', ins):
            ins = re.sub(r'\b([0-9a-f]+) ?$', lambda m: 'L%x' % (int(m.group(1), 16) - b), ins)
        out.append(ins)
    return out
tn = norm(t); on = norm(o)
sm = difflib.SequenceMatcher(None, tn, on, autojunk=False)
out = []; nd = 0
for tag, i1, i2, j1, j2 in sm.get_opcodes():
    if tag == 'equal':
        for k in range(i2 - i1):
            out.append("%4d %4d   %s" % (i1 + k, j1 + k, tn[i1 + k]))
    else:
        n = max(i2 - i1, j2 - j1); nd += n
        for k in range(n):
            a = tn[i1 + k] if i1 + k < i2 else ''
            b = on[j1 + k] if j1 + k < j2 else ''
            out.append("%4s %4s * %-34s | %s" % (i1 + k if i1 + k < i2 else '', j1 + k if j1 + k < j2 else '', a, b))
open(sys.argv[3], 'w').write("\n".join(out) + "\n")
print("%-10s diff=%d tgt=%d ours=%d li19=%d" % (sys.argv[4], nd, len(tn), len(on), sum(1 for x in on if x == 'li v0,19')))
