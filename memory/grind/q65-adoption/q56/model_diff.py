#!/usr/bin/env python3
"""model_diff.py: POC build (/tmp/q56/model) vs the oracle build's objects, per function.
For each differing function: which symbols gained gp (oracle lui/%lo -> model gp) and which lost it."""
import json, os, re, subprocess, sys
Q = "/tmp/q56"
MB = sys.argv[1] if len(sys.argv) > 1 else Q + "/model/build/src"

def funcs(o):
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases {o}", shell=True,
                         capture_output=True, text=True).stdout
    d = {}
    for m in re.finditer(r"^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=^[0-9a-f]+ <|\Z)", out, re.M | re.S):
        d[m.group(1)] = [re.sub(r"[0-9a-f]+ <[^>]+>", "T", re.sub(r"^\s*[0-9a-f]+:\s*", "", x)) for x in m.group(2).splitlines()]
    return d

def gp_syms(lines, kind):
    c = {}
    for l in lines:
        m = re.match(r"(R_MIPS_\w+)\s+(\S+)", l.strip())
        if m and m.group(1) == kind:
            c[m.group(2)] = c.get(m.group(2), 0) + 1
    return c

res = {}
bin_same = None
for o in sorted(os.listdir(Q + "/refobj")):
    a, b = f"{Q}/refobj/{o}", f"{MB}/{o}"
    if not os.path.exists(b):
        res[o] = "MISSING"; continue
    fa, fb = funcs(a), funcs(b)
    for fn in sorted(set(fa) | set(fb)):
        if fa.get(fn) == fb.get(fn):
            continue
        ga, gb = gp_syms(fa.get(fn, []), "R_MIPS_GPREL16"), gp_syms(fb.get(fn, []), "R_MIPS_GPREL16")
        gained = {s: gb[s] - ga.get(s, 0) for s in gb if gb[s] > ga.get(s, 0)}
        lost = {s: ga[s] - gb.get(s, 0) for s in ga if ga[s] > gb.get(s, 0)}
        res[f"{o[:-2]}:{fn}"] = {"gained_gp": gained, "lost_gp": lost,
                                 "len_ref": len(fa.get(fn, [])), "len_model": len(fb.get(fn, []))}
json.dump(res, open(Q + "/model_diff.json", "w"), indent=1)
for k, v in res.items():
    print(k, v)
print("differing functions:", len(res))
