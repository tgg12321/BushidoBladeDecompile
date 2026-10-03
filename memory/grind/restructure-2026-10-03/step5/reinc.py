"""reinc.py OLD NEW FILE... : replace every `#include OLD` line with `#include NEW` (NEW '-' deletes the line)."""
import sys
old, new, files = sys.argv[1], sys.argv[2], sys.argv[3:]
for p in files:
    t = open(p, encoding="utf-8", newline="").read()
    lines = t.split("\n")
    out = []
    for l in lines:
        if l.strip() == "#include " + old:
            if new != "-":
                out.append("#include " + new)
            continue
        out.append(l)
    n = "\n".join(out)
    if n != t:
        open(p, "w", encoding="utf-8", newline="\n").write(n)
    else:
        print("unchanged", p)
