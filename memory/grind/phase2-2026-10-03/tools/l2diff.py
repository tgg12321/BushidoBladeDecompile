#!/usr/bin/env python3
"""l2diff.py A [B] : layer-2 body keys (engine.layer2.body_key) of every function / stub in every
src/**/*.c whose text differs between snapshot A's source copy and snapshot B's (default: the working
tree). Prints one MOVED line per function whose key changed (file, function, old key, new key): each
names a landed body whose layer-2 review must be re-recorded. Exit 1 if any key moved."""
import os, sys
sys.path.insert(0, ".")
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from engine import layer2 as L2
from citems import items

A = f"tmp/p2/snap/{sys.argv[1]}/src"
B = f"tmp/p2/snap/{sys.argv[2]}/src" if len(sys.argv) > 2 else "."


def cfiles(root):
    out = set()
    for dp, _, fs in os.walk(os.path.join(root, "src")):
        for f in fs:
            if f.endswith(".c"):
                out.add(os.path.relpath(os.path.join(dp, f), root).replace(os.sep, "/"))
    return out


def read(root, f):
    try:
        return open(os.path.join(root, f), encoding="utf-8").read()
    except FileNotFoundError:
        return None


moved, nf, nfn = [], 0, 0
for f in sorted(cfiles(A) | cfiles(B)):
    old, new = read(A, f), read(B, f)
    if old == new:
        continue
    nf += 1
    if old is None or new is None:
        print("FILE", "added" if old is None else "removed", f)
        continue
    names = set()
    for t in (old, new):
        its, _ = items(t)
        names |= {i["name"] for i in its if i["kind"] in ("func", "stub") and i["name"]}
    for fn in sorted(names):
        nfn += 1
        ko, kn = L2.body_key(old, fn), L2.body_key(new, fn)
        if ko != kn:
            moved.append((f, fn, ko, kn))
for f, fn, ko, kn in moved:
    print("MOVED", f, fn, ko and ko[1], "->", kn and kn[1])
print(f"L2DIFF: {nf} changed files, {nfn} functions keyed, {len(moved)} moved keys")
sys.exit(1 if moved else 0)
