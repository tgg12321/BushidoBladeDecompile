#!/usr/bin/env python3
"""pdiff.py <old_dir> <new_dir> <out_dir>: compare the NN-*.patch files of two series runs, ignoring commit ids,
dates, blob ids and hunk line numbers; writes <out_dir>/NN.diff for each step that differs and prints a summary."""
import difflib, glob, os, re, sys

old, new, out = sys.argv[1:4]
os.makedirs(out, exist_ok=True)


def norm(p):
    keep = []
    for l in open(p, encoding="utf-8", errors="replace").read().split("\n"):
        if re.match(r"^(From [0-9a-f]{40} |Date: |index [0-9a-f]+\.\.[0-9a-f]+)", l):
            continue
        keep.append(re.sub(r"^@@ -\d+(,\d+)? \+\d+(,\d+)? @@", "@@", l))
    return keep


for nn in ["%02d" % i for i in range(1, 17)]:
    a = glob.glob(f"{old}/{nn}-*.patch")
    b = glob.glob(f"{new}/{nn}-*.patch")
    if not a or not b:
        print(f"step {nn}: missing ({len(a)} old, {len(b)} new)")
        continue
    d = list(difflib.unified_diff(norm(a[0]), norm(b[0]), os.path.basename(a[0]), os.path.basename(b[0]), n=2, lineterm=""))
    if d:
        open(f"{out}/{nn}.diff", "w", newline="\n").write("\n".join(d) + "\n")
        plus = sum(1 for l in d if l.startswith("+") and not l.startswith("+++"))
        minus = sum(1 for l in d if l.startswith("-") and not l.startswith("---"))
        print(f"step {nn}: differs (+{plus} -{minus}) -> {out}/{nn}.diff")
    else:
        print(f"step {nn}: identical (modulo ids / line numbers)")
