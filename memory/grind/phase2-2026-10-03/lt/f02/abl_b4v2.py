#!/usr/bin/env python3
# (F) FAKE ablation over F02 batch 4 v2 (v/17AFC.b4v2.c): func_800321E8's arg5++ / arg5-- pair,
# func_8002A458's do-while(0), func_8002CA8C's hit staging; each removed alone.
import sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b4 as T
import abl_b2 as A
base = open("tmp/p2/lt/f02/v/17AFC.b4v2.c", encoding="utf-8").read()
g = open("tmp/p2/lt/f02/v/game.h.b4v2", encoding="utf-8").read()
s3 = open("tmp/p2/lt/f02/v/3AB48.b4v2.c", encoding="utf-8").read()

def a321(s):
    i = s.index("                arg5++; /* FAKE:")
    j = s.index("\n", s.index("                arg5--;", i)) + 1
    return s[:i] + s[j:]

def a458(s):
    i = s.index("        /* FAKE: do-while(0) (do-while-zero-exception)")
    j = s.index("        } while (0);\n", i) + len("        } while (0);\n")
    return s[:i] + "        rec = D_800F5F68[id];\n" + s[j:]

# func_800321E8's +1-1 pair was retired with its arg5 local (whole-cluster ablation IDENTICAL; review
# rev-f02b4), so only the two remaining FAKEs are ablated here.
for name, f, fn in [("func_8002A458 do-while(0)", a458, "func_8002A458"),
                    ("func_8002CA8C hit staging", A.aca8c, "func_8002CA8C")]:
    print("==", name)
    T.measure(f(base), g, s3, [fn])
