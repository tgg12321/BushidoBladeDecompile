#!/usr/bin/env python3
"""one-off: insert the M3/M4 reconcile+merge steps as 08-11; old 08..12 become 12..16."""
import os, re, shutil
D = os.path.dirname(os.path.abspath(__file__))
for old in range(12, 7, -1):
    new = old + 4
    p, q = f"{D}/s{old:02d}_apply.py", f"{D}/s{new:02d}_apply.py"
    s = open(p, encoding="utf-8").read()
    s = re.sub(r'"""Step %d \(' % old, '"""Step %d (' % new, s, count=1)
    s = s.replace(f"usage: s{old:02d}_apply.py", f"usage: s{new:02d}_apply.py")
    s = s.replace(f'print("step {old} applied")', f'print("step {new} applied")')
    open(q, "w", newline="\n", encoding="utf-8").write(s)
    os.remove(p)
    print(p, "->", q)
for src, n in (("r3", 8), ("m3", 9), ("r4", 10), ("m4", 11)):
    s = open(f"{D}/{src}_apply.py", encoding="utf-8").read()
    s = re.sub(r'^"""(M[34] (?:reconciliation|merge))', r'"""Step %d: \1' % n, s, count=1)
    s = s.replace(f"usage: {src}_apply.py", f"usage: s{n:02d}_apply.py")
    open(f"{D}/s{n:02d}_apply.py", "w", newline="\n", encoding="utf-8").write(s)
    print(src, "->", n)
for f in os.listdir(D):
    if re.match(r"^(0[89]|1[0-2])-.*\.patch$", f):
        os.remove(f"{D}/{f}")
        print("removed", f)
