#!/usr/bin/env python3
# F01d-1 ablations on f01d1.py's output (tmp/p2/f01d1/51268.c): one candidate body per variant under
# tmp/p2/f01d1/abl/<name>.c + list.txt, scored by score_b2.py (sandbox recipe on the scratch copy).
# func_800678A8's pointer cluster (each pointer replaced by member accesses, and all five), and
# func_80065800's five FAKEs (F01a's abl_a.py variants, on F01d-1's text).
import os, re, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv; sys.argv = sys.argv[:1]
import f01d1 as D
sys.argv = _argv
NL = chr(10)
OUT = "tmp/p2/f01d1/abl"
os.makedirs(OUT, exist_ok=True)

def span(s, f):
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, s)
    i = m.start() + 1
    return i, s.index("\n}\n", i) + 3

def body(s, f):
    i, j = span(s, f)
    return s[i:j]

s0, g0, h0 = D.base()
S = D.src(s0)
assert S == open("tmp/p2/f01d1/51268.c", encoding="utf-8").read()
V = {}
for k in ("m678", "a678_p0", "a678_p2", "a678_p4", "a678_p6C", "a678_p80", "a678_new"):
    D.OPT = {k}
    V["800678A8_" + k] = ("func_800678A8", body(D.src(s0), "func_800678A8"))
D.OPT = set()

# func_80065800: F01a's variants
def rep(b, a, c, n=1):
    assert b.count(a) == n, (a[:80], b.count(a), n)
    return b.replace(a, c)
b0 = body(S, "func_80065800")
def drop_decl(b, var):
    for l in b.split("\n"):
        if re.match(r"\s+s(32|16) \*?%s; /\* FAKE" % var, l):
            i = b.index(l + "\n"); j = i + len(l) + 1
            if var == "tbl":
                j = b.index("*/\n", i) + 3
            return b[:i] + b[j:]
    raise KeyError(var)
b = drop_decl(b0, "w")
b = rep(b, "            w = *p_w;\n", "")
b = rep(b, "p_v->vx = -w;", "p_v->vx = -*p_w;"); b = rep(b, "p_v->vx = w;", "p_v->vx = *p_w;")
V["65800_w"] = ("func_80065800", b)
b = drop_decl(b0, "h")
b = rep(b, "            h = *p_h;\n", "")
b = rep(b, "p_v->vy = -h;", "p_v->vy = -*p_h;"); b = rep(b, "p_v->vy = h;", "p_v->vy = *p_h;")
V["65800_h"] = ("func_80065800", b)
b = drop_decl(b0, "sw")
b = rep(b, "        sw = (rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25;\n", "")
b = rep(b, "*p_w = (*p_w * sw >> 13) / 2;", "*p_w = (*p_w * ((rsin((D_800F0BA8[arg0] << 11) / 4551 - 0x400) + 0x1000) * 25) >> 13) / 2;")
V["65800_sw"] = ("func_80065800", b)
b = drop_decl(b0, "sh")
b = rep(b, "        sh = rsin((D_800F0BA8[arg0] << 11) / 4551) * 15;\n", "")
b = rep(b, "*p_h = (*p_h * sh >> 12) / 2;", "*p_h = (*p_h * (rsin((D_800F0BA8[arg0] << 11) / 4551) * 15) >> 12) / 2;")
V["65800_sh"] = ("func_80065800", b)
b = drop_decl(b0, "tbl")
b = rep(b, "        tbl = D_800F0BA8;\n        t = tbl + arg0;\n", "        t = D_800F0BA8 + arg0;\n")
V["65800_tbl"] = ("func_80065800", b)

with open(OUT + "/list.txt", "w", encoding="utf-8", newline=NL) as fl:
    for name, (f, b) in V.items():
        open("%s/%s.c" % (OUT, name), "w", encoding="utf-8", newline=NL).write(b)
        fl.write("%s %s\n" % (name, f))
print(len(V), "variants")
