"""l2cmp.py: layer-2 body keys of every function of the old main.c, old text vs its part now."""
import sys
sys.path.insert(0, ".")
from engine import layer2 as L2
old = open("tmp/r4f/old_main.c", encoding="utf-8").read()
funcs = [l.split()[2] for l in open("tmp/r4f/mainsyms.txt") if l.strip()]
funcs += ["DisableEvent", "WaitEvent"]
moved, n = [], 0
rows = []
for f in funcs:
    ko = L2.body_key(old, f)
    stem = L2.locate_stem(f)
    kn = L2.current_key(f, stem) if stem else None
    n += 1
    rows.append(f"{f} {ko} {kn} {stem}")
    if ko != kn:
        moved.append((f, ko, kn, stem))
open("tmp/r4f/l2_rows.txt", "w").write("\n".join(rows) + "\n")
print(n, "functions;", "moved:", moved)
