import sys, re, difflib
# s4cmp.py t.txt x.txt: instruction diff with relocation lines dropped and branch/jump targets masked
def norm(p):
    out = []
    for l in open(p):
        l = l.rstrip('\n')
        if 'R_MIPS' in l:
            continue
        l = re.sub(r'\b[0-9a-f]+ <[^>]*>', 'T', l)
        l = re.sub(r'(\t(b\w*|j|jal)\t.*?)([0-9a-f]+)$', r'\1T', l)
        out.append(l)
    return out
t, x = norm(sys.argv[1]), norm(sys.argv[2])
sm = difflib.SequenceMatcher(None, t, x, autojunk=False)
d = sum(max(i2 - i1, j2 - j1) for op, i1, i2, j1, j2 in sm.get_opcodes() if op != 'equal')
print(d)
