#!/usr/bin/env python3
# F01b1 ablations: each FAKE (or alias cluster) of a moved body removed alone. Base: the generated
# 51268.c (src/main/51268.c, f01b.py's output). Writes one candidate body per
# variant to tmp/p2/lt/f01/ablb/<name>.c (+ list.txt for run_abl_b.ps1, engine sandbox --candidate)
# and, with `measure`, compiles each whole-file variant in tmp/p2/wk and prints fcmp2's verdict.
import os, re, shutil, subprocess, sys
NL = chr(10)
S = open("src/main/51268.c", encoding="utf-8").read()   # the staged file (== f01b.py output)
OUT = "tmp/p2/lt/f01/ablb"
os.makedirs(OUT, exist_ok=True)

def span(s, f):
    m = re.search(r"\n[a-z0-9_]+ %s\(" % f, s)
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return i, j

def body(f):
    i, j = span(S, f)
    return S[i:j]

def rep(b, a, c, n=1):
    assert b.count(a) == n, (a[:80], b.count(a), n)
    return b.replace(a, c)

def drop_comment(b, start):
    a = b.index(start)
    e = b.index("*/\n", a) + 3
    return b[:a] + b[e:]

V = {}
# the FAKE &D_800F116C alias `v1` of func_80061658 / func_80061710, spelled directly (f01b.py already
# drops the alias where it is not needed: func_800618B4's FAKE one and 12 unannotated ones)
for f in ["func_80061658", "func_80061710"]:
    b = body(f)
    b = drop_comment(b, "    /* FAKE: local pointer alias to D_800F116C")
    b = rep(b, "    Unk1F800000Unk00 *v1 = &D_800F116C;\n", "")
    b = rep(b, "D_800A3468 = v1;", "D_800A3468 = &D_800F116C;")
    b = re.sub(r"\bv1->unk00\.w = ", "D_800F116C.unk00.w = ", b)
    assert not re.search(r"\bv1\b", b), f
    V[f[5:] + "_v1"] = (f, b)
# func_80063B34 / func_80065000: the word pointer -> a member read
b = body("func_80063B34")
b = drop_comment(b, "    /* FAKE: the word is read through an s32 *")
b = rep(b, "    s32 *v1 = &D_800A3468->unk00.w;\n", "")
b = rep(b, "(*v1 >> 17)", "(D_800A3468->unk00.w >> 17)")
V["80063B34_wptr"] = ("func_80063B34", b)
b = body("func_80063B34")
b = drop_comment(b, "    /* FAKE: the word is read through an s32 *")
b = rep(b, "    s32 *v1 = &D_800A3468->unk00.w;\n", "    Unk1F800000Unk00 *v1 = D_800A3468;\n")
b = rep(b, "(*v1 >> 17)", "(v1->unk00.w >> 17)")
V["80063B34_wptr_holder"] = ("func_80063B34", b)
b = body("func_80065000")
b = drop_comment(b, "    /* FAKE: the word is read through an s32 *")
b = rep(b, "    s32 *q = &D_800A3468->unk00.w;\n", "")
b = rep(b, "(*q >> 19)", "(D_800A3468->unk00.w >> 19)")
V["80065000_wptr"] = ("func_80065000", b)
b = body("func_80065000")
b = drop_comment(b, "    /* FAKE: the word is read through an s32 *")
b = rep(b, "    s32 *q = &D_800A3468->unk00.w;\n", "    Unk1F800000Unk00 *q = D_800A3468;\n")
b = rep(b, "(*q >> 19)", "(q->unk00.w >> 19)")
V["80065000_wptr_holder"] = ("func_80065000", b)

with open(OUT + "/list.txt", "w", encoding="utf-8", newline=NL) as fl:
    for name, (f, b) in V.items():
        open("%s/%s.c" % (OUT, name), "w", encoding="utf-8", newline=NL).write(b)
        fl.write("%s %s\n" % (name, f))

if "measure" in sys.argv[1:]:
    g = open("tmp/p2/lt/f01/v/game.h.b", encoding="utf-8").read()
    h = open("tmp/p2/lt/f01/v/bb2.h.b", encoding="utf-8").read()
    for name, (f, b) in V.items():
        i, j = span(S, f)
        s = S[:i] + b + S[j:]
        shutil.rmtree("tmp/p2/wk", ignore_errors=True)
        os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
        open("tmp/p2/wk/include/bb2.h", "w", encoding="utf-8", newline=NL).write(h)
        open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
        open("tmp/p2/wk/src/main/51268.c", "w", encoding="utf-8", newline=NL).write(s)
        r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268", f], capture_output=True, text=True)
        print(name, r.stdout.strip().replace(NL, " "))
print(len(V), "variants")
