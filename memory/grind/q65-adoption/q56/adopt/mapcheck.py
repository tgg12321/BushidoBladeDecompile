#!/usr/bin/env python3
"""mapcheck.py: the small-data area of the scratch build's link map vs the planned spans
(/tmp/q56/s04_spans.json): each C block's actual address and size against its planned start/end."""
import json, re
A = "/tmp/q56/adopt tree"
spans = {(f, s): (int(a, 16), int(e, 16)) for a, e, f, s in json.load(open("/tmp/q56/s04_spans.json"))["spans"]}
for l in open(A + "/build/bb2.map"):
    m = re.match(r"^ (\.\w+)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) build/(src|asm/data)/(\w+)\.o", l)
    if not m:
        continue
    a, n = int(m.group(2), 16), int(m.group(3), 16)
    if not (0x800A30C0 <= a < 0x800A3900):
        continue
    key = (m.group(5), m.group(1))
    plan = spans.get(key)
    flag = ""
    if plan:
        flag = "OK" if (a, a + n) == plan else f"PLAN {plan[0]:08x}..{plan[1]:08x}"
    print(f"{a:08x}..{a + n:08x} {m.group(1):7} {m.group(5):22} {flag}")
