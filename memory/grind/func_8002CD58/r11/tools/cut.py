"""Cut one function's section out of cc1 RTL dumps: python cut.py <dir> <func>  -> <dir>/<func>.<pass>"""
import sys
from pathlib import Path

d, func = Path(sys.argv[1]), sys.argv[2]
for p in ("flow", "lreg", "greg"):
    t = (d / f"tu.i.{p}").read_text(errors="replace")
    i = t.index(f";; Function {func}\n")
    j = t.find("\n;; Function ", i + 10)
    (d / f"{func}.{p}").write_text(t[i:j if j > 0 else None])
    print(p, len(t[i:j].splitlines()))
