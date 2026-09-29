#!/usr/bin/env python3
"""mkvar.py <base.c> <out.c> <old1> <new1> [<old2> <new2> ...]  -- literal substring replacement (all occurrences),
asserting each old string is present. Writes LF."""
import sys
base, out = sys.argv[1], sys.argv[2]
s = open(base, newline='\n').read()
pairs = sys.argv[3:]
for i in range(0, len(pairs), 2):
    old, new = pairs[i].replace('\\n', '\n'), pairs[i + 1].replace('\\n', '\n')
    assert old in s, 'missing: ' + old
    s = s.replace(old, new)
open(out, 'w', newline='\n').write(s)
