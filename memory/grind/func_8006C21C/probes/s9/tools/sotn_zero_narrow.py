#!/usr/bin/env python3
"""Find SOTN functions with a narrow local (s16/u16/s8/u8/short/char) whose every write is
the literal 0 and that is read at least once (a zero-valued narrow local)."""
import re, sys, pathlib
root = pathlib.Path('tmp/sotn-decomp/src')
NARROW = r'(?:s16|u16|s8|u8|short|unsigned short|signed short|char|unsigned char|signed char)'
decl_re = re.compile(r'^\s*(?:volatile\s+)?' + NARROW + r'\s+([^;()]+);', re.M)
func_re = re.compile(r'^[A-Za-z_][\w\s\*]*?\b(\w+)\s*\([^;{]*\)\s*\{', re.M)
hits = []
for f in sorted(root.rglob('*.c')):
    t = open(f, errors='replace').read()
    for m in func_re.finditer(t):
        start = m.end() - 1
        depth = 0; i = start
        while i < len(t):
            c = t[i]
            if c == '{': depth += 1
            elif c == '}':
                depth -= 1
                if depth == 0: break
            i += 1
        body = t[start:i + 1]
        for d in decl_re.finditer(body):
            for part in d.group(1).split(','):
                part = part.strip()
                mm = re.match(r'\*?\s*(\w+)\s*(?:=\s*(.+))?$', part)
                if not mm or part.startswith('*') or '[' in part: continue
                name, init = mm.group(1), mm.group(2)
                if init is not None and init.strip() not in ('0',): continue
                occ = [x.start() for x in re.finditer(r'\b%s\b' % re.escape(name), body)]
                if len(occ) < 2: continue
                writes = re.findall(r'\b%s\s*(=(?!=)[^;,)]*|\+\+|--|[-+*/|&^]=|<<=|>>=)' % re.escape(name), body)
                pre = re.findall(r'(?:\+\+|--)\s*%s\b|&\s*%s\b' % (re.escape(name), re.escape(name)), body)
                bad = [w for w in writes if not re.fullmatch(r'=\s*0\s*', w)]
                if bad or pre: continue
                nwr = len(writes) + (1 if init is not None else 0)
                reads = len(occ) - len(writes) - 1  # minus the declaration
                if nwr >= 1 and reads >= 1:
                    hits.append((str(f), m.group(1), name, d.group(0).strip(), nwr, reads))
for h in hits:
    print('%s:%s  %s  [%s] writes=%d reads=%d' % h)
print(len(hits), 'hits', file=sys.stderr)
