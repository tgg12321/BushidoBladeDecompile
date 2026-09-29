#!/usr/bin/env python3
"""mkst.py PVDIR -- structural respellings measured on the per-value spellings
(Ruling 11 (D)(4)). Reads PVDIR/pv_idx.c / pv_idx2.c and writes st_*.c."""
import os, sys
D = sys.argv[1]


def sub(s, old, new):
    assert s.count(old) == 1, (old[:70], s.count(old))
    return s.replace(old, new)


out = {}
a = open(os.path.join(D, "pv_idx.c"), encoding="utf-8").read()
# keyframe and column loops as while loops
w = sub(a, "            for (kidx = 0; kidx < 63; kidx++) {\n",
        "            kidx = 0;\n            while (kidx < 63) {\n")
w = sub(w, "                    GETBITS(work[kidx + 3], 12);\n                }\n            }\n",
        "                    GETBITS(work[kidx + 3], 12);\n                }\n                kidx++;\n            }\n")
w = sub(w, "        for (col = 0; col < 3; col++) {\n            x = *p;\n            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);\n            p++;\n        }\n",
        "        col = 0;\n        while (col < 3) {\n            x = *p;\n            *p = (x & 0x800) ? (x | ~0xFFF) : (x & 0xFFF);\n            p++;\n            col++;\n        }\n")
out["st_idx_while"] = w
# column index at function scope instead of the row-loop body
b = sub(a, "        s32 col;\n\n        for (col = 0; col < 3; col++) {\n",
        "        for (col = 0; col < 3; col++) {\n")
b = sub(b, "    s32 ch;\n", "    s32 col;\n    s32 ch;\n")
out["st_idx_colfn"] = b

c = open(os.path.join(D, "pv_idx2.c"), encoding="utf-8").read()
# row index declared in a block around the post-pass
r = sub(c, "    s32 row;\n", "")
r = sub(r, "    p = &work[0x36];\n    for (row = 0; row < 2; row++) {\n",
        "    p = &work[0x36];\n    {\n        s32 row;\n\n        for (row = 0; row < 2; row++) {\n")
r = sub(r, "        p += 3;\n    }\n", "        p += 3;\n        }\n    }\n")
out["st_idx2_rowblk"] = r
# row loop as a do-while
q = sub(c, "    for (row = 0; row < 2; row++) {\n", "    row = 0;\n    do {\n")
q = sub(q, "        p += 3;\n    }\n", "        p += 3;\n        row++;\n    } while (row < 2);\n")
out["st_idx2_dowhile"] = q
for n, t in out.items():
    with open(os.path.join(D, n + ".c"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write(t)
    print(os.path.join(D, n + ".c"))
