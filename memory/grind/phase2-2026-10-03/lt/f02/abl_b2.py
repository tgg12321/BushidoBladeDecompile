#!/usr/bin/env python3
# (F) whole-cluster FAKE ablation over F02 batch 2 (tmp/p2/lt/f02/v/17AFC.b2.c): each FAKE removed
# in turn, the TU rebuilt in tmp/p2/wk, the four bodies compared against the base snapshot.
import os, re, shutil, subprocess, sys
sys.path.insert(0, "tmp/p2/lt/f02")
from f02b2 import edit
NL = chr(10)
base = open("tmp/p2/lt/f02/v/17AFC.b2.c", encoding="utf-8").read()
g = open("tmp/p2/lt/f02/v/game.h.b2", encoding="utf-8").read()

def cut_comment(b, start):
    i = b.index(start); j = b.index("*/", i) + 2
    k = b.rindex(NL, 0, i)
    return b[:k] + b[j:]

def a320(s):
    i = s.index("            ret = 1; /* FAKE: dead store"); j = s.index("*/", i) + 3
    return s[:i] + s[j:]

def a780_flag(s):
    return edit(s, "func_8002D780", [
        ("                s32 by;\n", "                s32 by;\n                s32 dy;\n"),
        ("                flag = y2 - y0;\n", "                dy = y2 - y0;\n"),
        ("(flag * ax)", "(dy * ax)"), ("(flag * (px - x0))", "(dy * (px - x0))")])

def a780_m(s):
    i = s.index("                /* FAKE: same-value re-store of the local `m`")
    j = s.index("                m = cut_r_sq;\n", i) + len("                m = cut_r_sq;\n")
    return s[:i] + s[j:]

def a518(s):
    i = s.index("                    /* FAKE: redundant same-value re-store of the LOCAL `ud`")
    j = s.index("                    ud = disc;\n", i) + len("                    ud = disc;\n")
    return s[:i] + s[j:]

def aca8c(s):
    i = s.index("        /* FAKE: the AABB reject flag is staged"); j = s.index("*/", i) + 3
    s = s[:i] + s[j:]
    return edit(s, "func_8002CA8C", [
        ("        s32 hit;\n", "        s32 hit;\n        s32 reject;\n"),
        ("        hit = 0;\n        x = SPAD", "        reject = 0;\n        x = SPAD"),
        ("hit = 1;", "reject = 1;", 3),
        ("        if (hit != 0) {\n            continue;\n", "        if (reject != 0) {\n            continue;\n")])

ABL = [("func_8002D320 dead store", a320), ("func_8002D780 flag staging", a780_flag),
       ("func_8002D780 m re-store", a780_m), ("func_8002D518 ud re-store", a518),
       ("func_8002CA8C hit staging", aca8c)]
FS = ["func_8002D320", "func_8002D780", "func_8002D518", "func_8002CA8C"]
for name, f in (ABL[int(sys.argv[1]) if len(sys.argv) > 1 else 0:] if __name__ == "__main__" else []):
    src = f(base)
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/17AFC"] + FS, capture_output=True, text=True)
    print("==", name); print(r.stdout.strip())
