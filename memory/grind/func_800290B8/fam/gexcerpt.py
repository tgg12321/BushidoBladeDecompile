#!/usr/bin/env python3
"""gexcerpt.py <dumpdir>... : for the bounding-box loop's row (first `ashiftrt ... 1` set) and
column (first `and (reg 80) 1` set) pseudos, print the .lreg life line, any local-alloc seat,
and the .greg conflict / preference lines and final disposition."""
import re, sys
for d in sys.argv[1:]:
    lreg = open(d + '/f.lreg').read(); greg = open(d + '/f.greg').read()
    row = re.search(r'\(set \(reg/v:SI (\d+)\)\s+\(ashiftrt:SI \(reg:SI \d+\)\s+\(const_int 1\)', lreg).group(1)
    col = re.search(r'\(set \(reg/v:SI (\d+)\)\s+\(and:SI \(reg/v:SI 80\)\s+\(const_int 1\)', lreg).group(1)
    disp = dict(re.findall(r'(\d+) in (\d+)', greg[greg.index('dispositions'):]))
    print('=== %s  (row pseudo %s, column pseudo %s)' % (d, row, col))
    for name, p in (('row', row), ('column', col)):
        for l in lreg.splitlines():
            if l.startswith('Register %s used' % p) or re.fullmatch(r';; Register %s in \d+\.' % p, l):
                print('  %s .lreg: %s' % (name, l))
        for l in greg.splitlines():
            if l.startswith(';; %s conflicts:' % p) or l.startswith(';; %s preferences:' % p):
                print('  %s .greg: %s' % (name, l))
        print('  %s disposition: hard reg %s' % (name, disp.get(p, '(local-alloc / none)')))
