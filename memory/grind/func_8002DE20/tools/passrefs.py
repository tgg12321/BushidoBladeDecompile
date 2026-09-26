"""Count references to given pseudos in func_8002DE20 across the cc1 -da pass dumps.
usage: python3 passrefs.py <tag> <pseudo> [<pseudo>...]"""
import os
import re
import sys

tag, regs = sys.argv[1], sys.argv[2:]
d = f"tmp/rtl/de20_{tag}/"
order = ["rtl", "jump", "cse", "loop", "cse2", "flow", "combine", "lreg", "greg"]
for p in order:
    f = d + "in.i." + p
    if not os.path.exists(f):
        continue
    txt = open(f).read()
    m = re.search(r"\n;; Function func_8002DE20\n(.*?)(?=\n;; Function |\Z)", txt, re.S)
    sec = m.group(1) if m else ""
    counts = " ".join(f"{r}:{len(re.findall(rf'\(reg(?:/v)?:SI {r}\)', sec))}" for r in regs)
    print(f"{p:8s} {counts}")
