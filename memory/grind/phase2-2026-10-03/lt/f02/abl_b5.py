#!/usr/bin/env python3
# (F) FAKE ablation over F02 batch 5 (v/17AFC.b5.c): func_8002AB08's `vec` handle to D_800A37E8
# (the local and its comment removed, each use spelled &D_800A37E8).
import re, sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b5 as T
import f02b2 as B
s = open("tmp/p2/lt/f02/v/17AFC.b5.c", encoding="utf-8").read()
g = open("tmp/p2/lt/f02/v/game.h.b5", encoding="utf-8").read()
i, j = B.span(s, "func_8002AB08")
b = s[i:j]
a = b.index("    /* FAKE: second handle to D_800A37E8")
e = b.index("    s16 *vec = &D_800A37E8;\n") + len("    s16 *vec = &D_800A37E8;\n")
b = b[:a] + b[e:]
n = len(re.findall(r"\bvec\b", b))
b = re.sub(r"\bvec\b", "&D_800A37E8", b)
print("vec uses replaced:", n)
T.measure(s[:i] + b + s[j:], g)
