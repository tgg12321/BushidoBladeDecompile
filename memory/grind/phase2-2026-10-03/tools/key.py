import sys, os
sys.path.insert(0, "."); sys.path.insert(0, "memory/grind/phase2-2026-10-03/tools")
from engine import layer2 as L2
from citems import items
snap, f = sys.argv[1], sys.argv[2]
old = open(f"tmp/p2/snap/{snap}/src/{f}", encoding="utf-8").read(); new = open(f, encoding="utf-8").read()
names = set()
for t in (old, new):
    its, _ = items(t); names |= {i["name"] for i in its if i["kind"] in ("func","stub") and i["name"]}
import re
allf = set(re.findall(r'^(?:[A-Za-z_][\w \*]*?\s+\**)?(func_[0-9A-F]{8}|[A-Za-z_]\w*)\s*\([^;]*\)\s*\{?\s*$', new, re.M))
for fn in sorted(allf - names):
    ko, kn = L2.body_key(old, fn), L2.body_key(new, fn)
    if ko != kn: print("UNKEYED-MOVED", f, fn, ko and ko[1], "->", kn and kn[1])
