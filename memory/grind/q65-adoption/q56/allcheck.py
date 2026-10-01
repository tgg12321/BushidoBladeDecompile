#!/usr/bin/env python3
"""allcheck.py: (1) r5 full-build bin == incremental relink bin; (2) in the all-rows-removed full build,
which functions' disassembly differs from the reference (expect exactly the live-row functions)."""
import os, re, subprocess, json
Q = "/tmp/q56"
print("r5 full vs incremental relink:", open(Q+"/full/r5/build/bb2.bin","rb").read() == open(Q+"/link/r5/build/bb2.bin","rb").read())
def funcs(o):
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn {o}", shell=True, capture_output=True, text=True).stdout
    d = {}
    for m in re.finditer(r"^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=^[0-9a-f]+ <|\Z)", out, re.M | re.S):
        d[m.group(1)] = [re.sub(r"^\s*[0-9a-f]+:\s*", "", x) for x in m.group(2).splitlines()]
    return d
changed = set(); objs = set()
for o in sorted(os.listdir(Q+"/refobj")):
    a = open(f"{Q}/refobj/{o}","rb").read(); b = open(f"{Q}/full/all_removed/build/src/{o}","rb").read()
    if a == b: continue
    objs.add(o)
    fa, fb = funcs(f"{Q}/refobj/{o}"), funcs(f"{Q}/full/all_removed/build/src/{o}")
    for k in set(fa) | set(fb):
        if fa.get(k) != fb.get(k):
            # strip branch-target addresses (shift artifacts) before comparing
            na = [re.sub(r"[0-9a-f]+ <[^>]+>", "T", x) for x in fa.get(k, [])]
            nb = [re.sub(r"[0-9a-f]+ <[^>]+>", "T", x) for x in fb.get(k, [])]
            if na != nb: changed.add(k)
R = json.load(open(Q+"/results.json"))
live = set(R["rows"][n]["func"] for n, t in R["row_tests"].items() if not t.get("obj_identical", True))
live |= set(f for f, t in R["func_tests"].items() if not t.get("obj_identical", True))
print("objects differing:", len(objs))
print("functions changed:", len(changed), "live-row funcs:", len(live))
print("changed but not live:", sorted(changed - live))
print("live but not changed:", sorted(live - changed))
