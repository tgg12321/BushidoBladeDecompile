#!/usr/bin/env python3
"""Print score + scored (non-masked) hunks of a sandbox --diff output file. Optional 2nd arg: max hunks."""
import re, sys
t = open(sys.argv[1], encoding='utf-8', errors='replace').read()
lim = int(sys.argv[2]) if len(sys.argv) > 2 else 999
m = re.search(r'"score": (\d+)', t); b = re.search(r'"build_insns": (\d+)', t)
print('score', m and m.group(1), 'build', b and b.group(1))
n = 0
for h in re.split(r'\n(?=@ hunk)', t):
    if h.startswith('@ hunk') and 'not-scored' not in h.split('\n')[0]:
        n += 1
        if n <= lim:
            print(h.strip().split('\n  NOTE')[0]); print()
