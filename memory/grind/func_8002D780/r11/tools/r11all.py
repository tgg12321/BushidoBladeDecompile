"""All 64 split subsets of func_8002D780's six cross-product values -> tmp/func_8002D780/r11all/*.c.
Name: s_<c-split-values>_<p-split-values>, e.g. s_1_23 = cross_center value 1 split, cross_point values 2,3 split."""
import itertools
import os
import sys

sys.argv = [sys.argv[0]]
g = {}
exec(open("tmp/func_8002D780/r11gen.py", encoding="utf-8").read().split("out = [write")[0], g)
split, cleanup, ren = g["split"], g["cleanup"], g["ren"]
OUT = "tmp/func_8002D780/r11all"
os.makedirs(OUT, exist_ok=True)
paths = []
items = [("c", 1), ("c", 2), ("c", 3), ("p", 1), ("p", 2), ("p", 3)]
for mask in range(64):
    chosen = [it for k, it in enumerate(items) if mask >> k & 1]
    t = ren
    for var, val in chosen:
        t = split(t, var, val)
    t = cleanup(t)
    cs = "".join(str(v) for var, v in chosen if var == "c") or "0"
    ps = "".join(str(v) for var, v in chosen if var == "p") or "0"
    p = f"{OUT}/s_{cs}_{ps}.c"
    open(p, "w", encoding="utf-8", newline="\n").write(t)
    paths.append(p)
open("tmp/func_8002D780/r11all_list.txt", "w", encoding="utf-8", newline="\n").write(",".join(paths))
print(len(paths))
