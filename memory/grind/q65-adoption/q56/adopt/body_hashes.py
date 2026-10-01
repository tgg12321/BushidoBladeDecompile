#!/usr/bin/env python3
"""body_hashes.py <tree>: scratch clone only (read-only, via git show). For each tag step00(=step0)..step16,
the layer-2 body key of every C function defined in src/*.c; prints, per step, every function whose C body key
changed versus the previous step (old -> new), plus functions that appear/disappear as C."""
import re, subprocess, sys
sys.path.insert(0, sys.argv[1])
from engine import layer2  # noqa: E402

A = sys.argv[1]
DEF = re.compile(r"^[A-Za-z_][\w \t\*]*?\b([A-Za-z_]\w*)\s*\([^;{]*\)\s*\{?\s*$", re.M)


def git(*a):
    return subprocess.run(["git", "-C", A, *a], capture_output=True, text=True).stdout


def keys_at(tag):
    out = {}
    files = [f for f in git("ls-tree", "--name-only", f"{tag}:src").split() if f.endswith(".c")]
    for f in files:
        text = git("show", f"{tag}:src/{f}")
        names = set(DEF.findall(text))
        for n in names:
            if n in ("if", "while", "for", "switch", "return", "sizeof"):
                continue
            src = layer2.body_source(text, n, read=lambda p: None)
            if src and src[0] == "c":
                out[n] = (layer2._key(src[1]), f)
    return out


tags = ["step0"] + [f"step{n:02d}" for n in range(1, 17)]
prev = keys_at(tags[0])
print(f"step0: {len(prev)} C bodies")
for t in tags[1:]:
    cur = keys_at(t)
    ch = []
    for n in sorted(set(prev) | set(cur)):
        a, b = prev.get(n), cur.get(n)
        if a is None or b is None:
            ch.append(f"  {n}: {'(none)' if a is None else a[0] + ' ' + a[1]} -> {'(none)' if b is None else b[0] + ' ' + b[1]}")
        elif a[0] != b[0]:
            ch.append(f"  {n}: {a[0]} -> {b[0]} ({b[1]})")
    print(f"{t}: {len(ch)} body change(s)")
    print("\n".join(ch)) if ch else None
    prev = cur
