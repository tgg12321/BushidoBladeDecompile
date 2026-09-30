#!/usr/bin/env python3
"""Score permuter outputs with the q.py harness: take each output's source.c from the
ScrPad typedef to the end (the permuted function plus its helpers), append it to the
code6cac_b.c prefix, and report q.py's line."""
import os, re, subprocess, sys, glob
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
os.chdir(ROOT)
P = "tmp/func_80029454/r11/perm"
dirs = sorted(glob.glob(f"{P}/output-*"), key=lambda d: int(d.split("-")[-2]))
for d in dirs:
    s = open(f"{d}/source.c").read()
    # the typedef block that precedes `} ScrPad;`
    end = s.find("} ScrPad;")
    start = s.rfind("typedef struct", 0, end)
    body = s[start:]
    out = f"{d}/tail.c"
    open(out, "w").write(body)
    r = subprocess.run(["bash", "tmp/func_80029454/q.sh", out], capture_output=True, text=True)
    print(d.split("/")[-1], (r.stdout.strip().splitlines() or [r.stderr.strip()[-200:]])[0])
