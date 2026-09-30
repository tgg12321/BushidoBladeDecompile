"""Print func_8002D780's instructions from a cc1 .s (tu.s) in <dir>, numbered, labels kept."""
import sys

t = open(f"{sys.argv[1]}/tu.s", encoding="utf-8").read().split("\n")
i = next(k for k, l in enumerate(t) if l.startswith("func_8002D780:"))
n = 0
for l in t[i:]:
    if l.startswith("\t.end\tfunc_8002D780"):
        break
    s = l.strip()
    if not s or s.startswith(".") and not s.endswith(":") or s.startswith("#"):
        continue
    if s.endswith(":"):
        print(s)
        continue
    print(f"{n:4d}  {s}")
    n += 1
