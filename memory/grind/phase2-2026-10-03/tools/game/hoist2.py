# hoist2.py ROOT DEFTU SYM... : declare each SYM (a function defined in src/main/<DEFTU>.c) once in
# ROOT/include/bb2.h's sorted multi-TU function block, spelled as its definition (parameter names
# dropped), and delete every one-line declaration of it from ROOT/src/main/*.c.
# ROOT is "." (the tree) or "tmp/p2/wk" (work copies; missing files are copied from the tree first).
import glob, os, re, shutil, sys

root, tu, syms = sys.argv[1], sys.argv[2], sys.argv[3:]
def P(p):
    if root == ".": return p
    w = os.path.join(root, p)
    if root != "." and not os.path.exists(w):
        os.makedirs(os.path.dirname(w), exist_ok=True); shutil.copy(p, w)
    return w
def rd(p): return open(P(p), "rb").read().decode("utf-8")
def wr(p, s):
    import time
    for k in range(20):
        try:
            with open(P(p), "wb") as fh: fh.write(s.encode("utf-8"))
            return
        except OSError:
            time.sleep(0.5)
    raise SystemExit("write failed: " + p)

STMT = re.compile(r"^\s*(return|else|do|case|goto|static|if|while|typedef)\b")
EXPR = re.compile(r"[-+&|=<>!/%\"']|\(\s*\d|,\s*\d")

def proto(name):
    s = rd(f"src/main/{tu}.c")
    m = re.search(r"^(?!static)([A-Za-z_][\w \*]*?[\s\*])" + name + r"\s*\(([^;{}()]*)\)\s*((?:[^;{}()]+;\s*)*)\{", s, re.M)
    assert m, name
    ret = re.sub(r"\s+", " ", m.group(1)).strip()
    ret = re.sub(r"\s*(\*+)$", lambda mm: " " + mm.group(1), ret)
    params = m.group(2).strip()
    if m.group(3).strip():  # K&R
        kr = dict()
        for d in re.findall(r"([^;]+);", m.group(3)):
            mm = re.match(r"\s*(.*?)(\w+)\s*$", d)
            kr[mm.group(2)] = mm.group(1).strip()
        ps = [kr[p.strip()] for p in params.split(",")] if params else []
    elif params in ("", "void"):
        ps = ["void"]
    else:
        ps = []
        for p in params.split(","):
            p = p.strip()
            mm = re.match(r"^(.*?[\s\*])(\w+)$", p)
            ps.append(re.sub(r"\s+", " ", mm.group(1)).strip() if mm else p)
    sp = "" if ret.endswith("*") else " "
    return f"extern {ret}{sp}{name}({', '.join(ps)});"

h = rd("include/bb2.h").split("\n")
start = [i for i, l in enumerate(h) if re.match(r"extern s32 \*?g_player_ptrs\[\];", l)][0] + 2
end = start
while h[end].strip():
    end += 1
for n in syms:
    pr = proto(n)
    pat = re.compile(r"^\s*(extern\s+)?(const\s+)?[A-Za-z_]\w*(\s+[A-Za-z_]\w*)*[\s\*]+" + re.escape(n)
                     + r"\s*\(([^;{}]*)\)\s*;\s*(/\*.*\*/)?\s*$")
    for f in sorted(glob.glob("src/main/*.c")):
        f = f.replace("\\", "/")
        if '#include "bb2.h"' not in rd(f):
            continue
        t = rd(f).split("\n")
        keep = []
        for i, ln in enumerate(t):
            m = pat.match(ln)
            if m and not STMT.match(ln) and not EXPR.search(m.group(4)):
                print(f"{f}:{i+1}: -{ln.strip()}")
            else:
                keep.append(ln)
        if len(keep) != len(t):
            wr(f, "\n".join(keep))
    if any(l.startswith("extern") and re.search(r"\b" + n + r"\(", l) for l in h):
        print(f"bb2.h already declares {n}: " + [l for l in h if re.search(r"\b" + n + r"\(", l)][0]); continue
    def key(j):  # a comment line sorts with the declaration it precedes
        while not h[j].startswith("extern"):
            j += 1
        return re.search(r"(\w+)\(", h[j]).group(1)
    i = start
    while i < end and key(i) < n:
        i += 1
    h.insert(i, pr); end += 1
    print(f"bb2.h: +{pr}")
wr("include/bb2.h", "\n".join(h))
