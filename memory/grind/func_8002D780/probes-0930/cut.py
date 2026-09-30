"""Cut the func_8002D780 section out of each dump in <dir>; print lines matching the given regexes."""
import re
import sys

d = sys.argv[1]
pats = [re.compile(p) for p in sys.argv[2:]]
for f in ["tu.i.flow", "tu.i.combine", "tu.i.lreg", "tu.i.greg"]:
    t = open(f"{d}/{f}", encoding="utf-8").read()
    i = t.index(";; Function func_8002D780")
    j = t.find(";; Function", i + 10)
    sec = t[i:j if j >= 0 else len(t)]
    open(f"{d}/{f}.D780", "w", encoding="utf-8", newline="\n").write(sec)
    for n, line in enumerate(sec.split("\n"), 1):
        if any(p.search(line) for p in pats):
            print(f"{f}.D780:{n}: {line[:200]}")
