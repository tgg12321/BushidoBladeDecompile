#!/usr/bin/env python3
"""conflicts.py <build.log>: for each `conflicting types for X` error in a merged file, print both
declaration lines (the new one and the previous one) from the scratch clone's source."""
import re, sys
A = "/tmp/q56/adopt tree"
log = open(sys.argv[1]).read().split("\n")
seen = set()
for i, l in enumerate(log):
    m = re.match(r"(src/\w+\.c):(\d+): conflicting types for `(\w+)'", l)
    if not m:
        continue
    f, ln, name = m.group(1), int(m.group(2)), m.group(3)
    pm = re.match(r"(src/\w+\.c):(\d+): previous declaration of", log[i + 1]) if i + 1 < len(log) else None
    if (f, name) in seen:
        continue
    seen.add((f, name))
    src = open(f"{A}/{f}").read().split("\n")
    new = src[ln - 1].strip()
    old = src[int(pm.group(2)) - 1].strip() if pm else "?"
    print(f"{f}  {name}\n    prev L{pm.group(2) if pm else '?'}: {old}\n    new  L{ln}: {new}")
