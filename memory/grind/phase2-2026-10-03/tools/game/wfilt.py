import sys, re
out = []
for l in sys.stdin:
    l = l.rstrip("\n")
    if re.search(r"missing terminating|In function|At top level", l) or not re.search(r"\.[ch]:\d+: ", l):
        continue
    out.append(re.sub(r"^[^:]*\.c:\d+: ", "", re.sub(r"^\S*/include/", "include/", l)))
print("\n".join(sorted(out)))
