"""Variant generator: python gen.py base.c out.c 'old1' 'new1' ['old2' 'new2' ...]
Literal replacements; each old must occur exactly once. \\n in args means newline."""
import sys

base, out = sys.argv[1], sys.argv[2]
pairs = sys.argv[3:]
src = open(base, encoding='utf-8').read()
for i in range(0, len(pairs), 2):
    old = pairs[i].replace('\\n', '\n')
    new = pairs[i + 1].replace('\\n', '\n')
    n = src.count(old)
    if n != 1:
        sys.exit('ERROR: %r occurs %d times' % (old, n))
    src = src.replace(old, new)
with open(out, 'w', encoding='utf-8', newline='\n') as f:
    f.write(src)
print('wrote', out)
