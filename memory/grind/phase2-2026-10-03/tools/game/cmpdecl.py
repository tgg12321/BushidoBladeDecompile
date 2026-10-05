# cmpdecl.py : for each removed declaration line in `git diff -- src` (worktree vs HEAD), compare its type spelling with the
# bb2.h prototype added in the same diff; print mismatches and a per-symbol file list.
import re, subprocess, collections
d = subprocess.run(["git","diff","-U0","HEAD","--","src","include/bb2.h"],capture_output=True,text=True).stdout
cur = None; rem = collections.defaultdict(list); add = {}
def norm(t):
    t = re.sub(r"/\*.*?\*/", "", t).replace("extern ", "").strip().rstrip(";")
    m = re.match(r"(.*?)(\w+)\s*\((.*)\)\s*$", t)
    if not m: return None, t
    ret, name, ps = m.groups()
    out = []
    for p in ps.split(","):
        p = p.strip()
        mm = re.match(r"^(.*?[\s\*])(\w+)$", p)
        if mm and mm.group(2) not in ("void",) and mm.group(1).strip() not in ("",): p = mm.group(1)
        out.append(re.sub(r"\s+", " ", p.replace(" *", "*").replace("*", " *")).strip())
    return name, re.sub(r"\s+", " ", ret.replace("*", " *")).strip() + " (" + ", ".join(out) + ")"
for l in d.split("\n"):
    if l.startswith("+++ "): cur = l[6:]
    elif l.startswith("-") and not l.startswith("---"):
        n, s = norm(l[1:])
        if n: rem[n].append((cur.split("/")[-1], s))
    elif l.startswith("+") and cur == "include/bb2.h" and l.startswith("+extern"):
        n, s = norm(l[1:]); add[n] = s
for n in sorted(add):
    fs = ", ".join(f for f, s in rem.get(n, []))
    bad = [(f, s) for f, s in rem.get(n, []) if s != add[n]]
    print(f"{n}: {add[n]}  <- {fs}" + ("".join(f"\n    MISMATCH {f}: {s}" for f, s in bad)))
