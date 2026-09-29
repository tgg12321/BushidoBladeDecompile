#!/usr/bin/env python3
"""Consumer table for the bytes 0x8009BD24..0x8009BD3B (D_8009BD24 record table + D_8009BD38 flag word)."""
import re, glob, os
out = []
inc = {}
for f in glob.glob('src/*.c'):
    for m in re.finditer(r'INCLUDE_ASM\("asm/funcs", (\w+)\)', open(f, errors='replace').read()):
        inc[m.group(1)] = f
out.append('## asm/funcs references (symbol forms), status of the function')
for f in sorted(glob.glob('asm/funcs/*.s')):
    t = open(f, errors='replace').read()
    hits = sorted(set(re.findall(r'D_8009BD(?:2[4-9A-F]|3[0-9A-F])(?: \+ 0x[0-9A-F]+)?', t)))
    if hits:
        fn = os.path.basename(f)[:-2]
        st = 'INCLUDE_ASM (' + inc[fn] + ')' if fn in inc else 'C'
        out.append('%-16s %-30s %s' % (fn, st, ', '.join(hits)))
out.append('')
out.append('## src C references (file:line: text)')
for f in sorted(glob.glob('src/*.c')) + sorted(glob.glob('include/*.h')):
    for n, line in enumerate(open(f, errors='replace'), 1):
        if re.search(r'D_8009BD(24|38)\b|D_800A3568|func_80077D00|arg2 \+ 0x14', line):
            out.append('%s:%d: %s' % (f, n, line.strip()))
open('memory/grind/func_8005E54C/consumers_raw.txt', 'w', newline='\n').write('\n'.join(out) + '\n')
print('\n'.join(out))
