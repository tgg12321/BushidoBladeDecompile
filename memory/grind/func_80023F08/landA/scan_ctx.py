import re
rows = [l for l in open('tmp/func_80023F08/scan.txt') if not l.startswith('#') and ('WIDTH' in l or 'SIGN' in l)]
funcre = re.compile(r'^\w[\w\s\*]*\b(\w+)\s*\(([^;]*)\)\s*\{?\s*$')
cache = {}
for l in rows:
    f, n = l.split()[0], int(l.split()[1])
    if '(scr +' in l:
        continue
    if f not in cache:
        cache[f] = open(f, encoding='utf-8', errors='replace').read().split('\n')
    L = cache[f]
    fn = '?'
    for i in range(n - 1, -1, -1):
        m = funcre.match(L[i])
        if m and not L[i].startswith('extern') and not L[i].startswith('typedef'):
            fn = m.group(1) + '(' + m.group(2)[:50] + ')'
            break
    h = re.search(r'\((?:\(u8 \*\))?(\w+) \+ 0x', l).group(1)
    decl = ''
    for i in range(n - 1, max(0, n - 400), -1):
        if re.search(r'\b' + h + r'\b\s*(=|;|,|\))', L[i]) and re.search(r'(u8|s32|PracticeMenuRec|void|s16|Obj\w*|\w+)\s*\*?\s*' + h + r'\b', L[i]):
            decl = L[i].strip()[:70]
            break
    print('%-22s %5d %-26s h=%-10s %-6s %s | decl: %s' % (f[4:], n, fn[:26], h, l.split()[-1], l.split()[2][:30], decl))
