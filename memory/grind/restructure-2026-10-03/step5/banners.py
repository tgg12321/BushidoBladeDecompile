"""banners.py FILE... : drop a declaration-section banner comment (one line: 'Declarations from ...',
'Forward declarations', 'Externs for globals', 'Named globals', 'Functions', 'Data symbols') that
is followed by a blank line (nothing left under it), together with that blank line."""
import re, sys
PAT = re.compile(r"\n/\* (Declarations from [^\n]*|Forward declarations|Externs for globals|Extern data declarations|Extern function declarations|Named globals|Functions|Data symbols) \*/\n\n")
for p in sys.argv[1:]:
    t = open(p, encoding="utf-8", newline="").read()
    n = PAT.sub("\n\n", t)
    while "\n\n\n" in n and n != t:
        # only collapse the triple newline the substitution itself produced
        i = n.find("\n\n\n")
        n = n[:i] + n[i + 1:]
        if n.count("\n\n\n") <= t.count("\n\n\n"):
            break
    if n != t:
        open(p, "w", encoding="utf-8", newline="\n").write(n)
        print("fixed", p)
