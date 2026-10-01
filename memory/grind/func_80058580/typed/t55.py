#!/usr/bin/env python3
"""t55.py <out-root>: Ruling 11 twins of func_80055138's temp / idx on the typed body (typed/f55138.c).
Line ranges are 1-based inclusive in typed/f55138.c."""
import os, re, sys, shutil
L = "memory/grind/func_80058580"
src = open(L + "/typed/f55138.c", encoding="utf-8").read().split("\n")
VALS = {
    "temp": {"lvl5": (50, 51), "lvl3": (59, 68), "row": (84, 88), "cmask": (132, 133), "b1": (138, 140), "b2": (144, 150)},
    "idx": {"clr": (104, 105), "who": (108, 126)},
}


def split(lines, items):
    out = list(lines)
    decl = []
    for var, val in items:
        a, b = VALS[var][val]
        for i in range(a - 1, b):
            out[i] = re.sub(r"\b%s\b" % var, val + "_", out[i])
        decl.append("    s32 %s_;" % val)
    k = out.index("    s32 temp;")
    return out[:k + 1] + decl + out[k + 1:]


root = sys.argv[1]
sets = {"pv_temp": [("temp", v) for v in VALS["temp"]], "pv_idx": [("idx", v) for v in VALS["idx"]]}
sets["pv_both"] = sets["pv_temp"] + sets["pv_idx"]
for var in VALS:
    for v in VALS[var]:
        sets["r_" + v] = [(var, v)]
for name, items in sets.items():
    d = os.path.join(root, name)
    os.makedirs(d, exist_ok=True)
    open(os.path.join(d, "f55138.c"), "w", encoding="utf-8", newline="\n").write("\n".join(split(src, items)))
    for f in ("f56fe8.c", "src_edits.py", "hdr_edits.py"):
        shutil.copy(L + "/typed/" + f, d)
    shutil.copy(L + "/candidate.c", os.path.join(d, "f58580.c"))
print(" ".join(sets))
