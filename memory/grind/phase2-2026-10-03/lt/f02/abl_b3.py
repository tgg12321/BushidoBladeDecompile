#!/usr/bin/env python3
# (F) FAKE ablation over F02 batch 3 (v/17AFC.b3.c): func_8002A458's do-while(0), unwrapped.
import sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b3 as T
s = open("tmp/p2/lt/f02/v/17AFC.b3.c", encoding="utf-8").read()
g = open("tmp/p2/lt/f02/v/game.h.b3", encoding="utf-8").read()
i = s.index("        /* FAKE: do-while(0) (do-while-zero-exception)")
j = s.index("        } while (0);\n", i) + len("        } while (0);\n")
s = s[:i] + "        rec = D_800F5F68[id];\n" + s[j:]
T.measure(s, g, ["func_8002A458"])
