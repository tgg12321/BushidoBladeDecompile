# fnat.py FILE PATTERN : print the enclosing function of each line matching PATTERN
import re, sys
f, pat = sys.argv[1], sys.argv[2]
cur = "?"
for i, l in enumerate(open(f, encoding="utf-8")):
    m = re.match(r"^[A-Za-z_][\w \*]*?[\s\*](\w+)\s*\([^;]*$", l)
    if m and not l.startswith(("extern", "typedef", "static ")) and m.group(1) not in ("if", "while"):
        cur = m.group(1)
    if re.search(pat, l):
        print(f"{i+1}: {cur}: {l.strip()[:100]}")
