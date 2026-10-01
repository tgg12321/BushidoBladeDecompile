#!/bin/bash
# where.sh: test clone - size and first differing addresses of build/bb2.exe vs the original, with the map symbol
# at or before each.
cd /tmp/q56r2/t || exit 1
ls -l build/bb2.exe | awk '{print "size", $5}'
cmp -l build/bb2.exe disc/SLUS_006.63 2>/dev/null | head -400 | awk '{printf "%x\n", $1 - 1 - 0x800 + 0x80010000}' | sort -u > /tmp/q56r2/diffaddrs.txt
wc -l < /tmp/q56r2/diffaddrs.txt
python3 - <<'PY'
import re, bisect
syms = []
for l in open("build/bb2.map"):
    m = re.match(r"^\s+0x0*([0-9a-f]{8})\s+([A-Za-z_]\w*)\s*$", l)
    if m: syms.append((int(m.group(1), 16), m.group(2)))
syms.sort()
keys = [a for a, _ in syms]
seen = []
for l in open("/tmp/q56r2/diffaddrs.txt"):
    a = int(l, 16)
    k = bisect.bisect_right(keys, a) - 1
    s = syms[k][1] if k >= 0 else "?"
    if s not in seen:
        seen.append(s)
        print(f"{a:#x} in {s} (+{a - syms[k][0]:#x})")
    if len(seen) > 15: break
PY
