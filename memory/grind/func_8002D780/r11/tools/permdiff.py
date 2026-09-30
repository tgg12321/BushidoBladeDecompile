"""Summarize permuter finds: for each output-* dir in <ws>, the changed lines of func_8002D780 vs base.c."""
import difflib
import glob
import os
import sys


def body(p):
    t = open(p, encoding="utf-8").read()
    i = t.index("func_8002D780(")
    return [l.rstrip() for l in t[i:].split("\n") if "b64literal" not in l and "__asm__" not in l]


ws = sys.argv[1]
base = body(sys.argv[2] if len(sys.argv) > 2 else f"{ws}/base.c")
outs = sorted(glob.glob(f"{ws}/output-*"), key=lambda p: (int(os.path.basename(p).split("-")[1]), p))
for o in outs:
    b = body(f"{o}/source.c")
    d = [l for l in difflib.unified_diff(base, b, lineterm="", n=0) if l[:1] in "+-" and l[:3] not in ("+++", "---")]
    print(f"== {os.path.basename(o)} ({len(d)} changed lines)")
    for l in d[:14]:
        print("   ", l[:130])
