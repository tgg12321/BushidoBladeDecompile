"""l2diff.py [REV] : layer-2 body keys of every function defined in every src/**/*.c changed since REV
(default HEAD), old text (git show REV:path) vs working tree. Prints moved keys; exit 1 if any moved."""
import json, re, subprocess, sys
sys.path.insert(0, ".")
sys.path.insert(0, "tmp/s5")
from engine import layer2 as L2
from citems import items
rev = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
files = subprocess.run(["git", "diff", "--name-only", rev, "--", "src"], capture_output=True, text=True).stdout.split()
files = [f for f in files if f.endswith(".c")]
moved, n = [], 0
for f in files:
    old = subprocess.run(["git", "show", f"{rev}:{f}"], capture_output=True, text=True, encoding="utf-8").stdout
    try:
        new = open(f, encoding="utf-8").read()
    except FileNotFoundError:
        continue
    names = set()
    for t in (old, new):
        its, _ = items(t)
        names |= {i["name"] for i in its if i["kind"] in ("func", "stub") and i["name"]}
    for fn in sorted(names):
        ko, kn = L2.body_key(old, fn), L2.body_key(new, fn)
        n += 1
        if ko != kn:
            moved.append((f, fn, ko, kn))
for m in moved:
    print("MOVED", *m)
print(f"{len(files)} files, {n} functions, {len(moved)} moved keys")
sys.exit(1 if moved else 0)
