"""l2all.py OUT FUNCLIST : layer-2 key (kind, hash, stem) of every function, one process."""
import sys
sys.path.insert(0, ".")
from engine import layer2 as L2
out, lst = sys.argv[1], sys.argv[2]
rows = []
for f in open(lst).read().split():
    stem = L2.locate_stem(f)
    k = L2.current_key(f, stem) if stem else None
    rows.append(f"{f} {k[1] if k else '-'} {k[0] if k else '-'} {stem}")
open(out, "w").write("\n".join(rows) + "\n")
print(len(rows), "rows;", sum(1 for r in rows if " - " in r), "without key")
