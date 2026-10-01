#!/usr/bin/env python3
"""one-off: after inserting the new step 3, bump 'Step N' / 'sNN_apply.py' / 'step N applied' in s04..s12."""
import re
for new in range(4, 13):
    old = new - 1
    p = f"tmp/q56/adopt/s{new:02d}_apply.py"
    s = open(p, encoding="utf-8").read()
    s = re.sub(r'"""Step %d \(' % old, '"""Step %d (' % new, s, count=1)
    s = s.replace(f"usage: s{old:02d}_apply.py", f"usage: s{new:02d}_apply.py")
    s = s.replace(f'print("step {old} applied")', f'print("step {new} applied")')
    open(p, "w", newline="\n", encoding="utf-8").write(s)
    print(p, re.search(r'"""Step \d+', s).group(0))
