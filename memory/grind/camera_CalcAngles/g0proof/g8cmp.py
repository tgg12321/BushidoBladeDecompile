import re, sys, difflib
O = sys.argv[1]

def fns(p):
    d, order, cur = {}, [], None
    for l in open(p):
        m = re.match(r"^[0-9a-f]+ <([^>]+)>:", l)
        if m:
            cur = m.group(1); d[cur] = []; order.append(cur); continue
        if cur and l.strip() and not l.startswith("Disassembly"):
            d[cur].append(re.sub(r"^[0-9a-f]+: ", "", re.sub(r"[0-9a-f]+ <[^>]+>", "T", l.strip())))
    return d, order

a, oa = fns(O + "/g0.dis")
b, ob = fns(O + "/g8.dis")
diff = [f for f in sorted(set(a) | set(b)) if a.get(f) != b.get(f) and not f.startswith(".L")]
print(len(diff), "functions differ (order-free):")
for f in diff:
    print(" ", f, len(a.get(f, [])), "->", len(b.get(f, [])))
    for x in list(difflib.unified_diff(a.get(f, []), b.get(f, []), n=0, lineterm=""))[2:16]:
        print("     ", x)
fa = [f for f in oa if not f.startswith(".L")]
fb = [f for f in ob if not f.startswith(".L")]
print("function order identical:", fa == fb, "| first divergence:",
      next(((i, x, y) for i, (x, y) in enumerate(zip(fa, fb)) if x != y), None))
print("--- sections -G0"); print(open(O + "/g0.h").read())
print("--- sections -G8"); print(open(O + "/g8.h").read())
