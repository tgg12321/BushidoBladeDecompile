#!/usr/bin/env python3
"""objdiff.py: per-function .text diff of the scratch build's objects vs /tmp/q56/pre04obj (the pre-switch
oracle objects): which symbols gained / lost a gp access. Objects that no longer exist are skipped."""
import os, re, subprocess
A = "/tmp/q56/adopt tree/build/src"
B = "/tmp/q56/pre04obj"

def funcs(o):
    out = subprocess.run(f"mipsel-linux-gnu-objdump -dr --no-show-raw-insn -M no-aliases -j .text '{o}'", shell=True,
                         capture_output=True, text=True).stdout
    d = {}
    for m in re.finditer(r"^[0-9a-f]+ <([^>]+)>:\n(.*?)(?=^[0-9a-f]+ <|\Z)", out, re.M | re.S):
        d[m.group(1)] = [re.sub(r"[0-9a-f]+ <[^>]+>", "T", re.sub(r"^\s*[0-9a-f]+:\s*", "", x)) for x in m.group(2).splitlines()]
    return d

def gp(lines):
    c = {}
    for l in lines:
        m = re.match(r"R_MIPS_GPREL16\s+(\S+)", l.strip())
        if m:
            c[m.group(1)] = c.get(m.group(1), 0) + 1
    return c

n = 0
for o in sorted(os.listdir(B)):
    if not o.endswith(".o") or not os.path.exists(f"{A}/{o}"):
        continue
    fa, fb = funcs(f"{B}/{o}"), funcs(f"{A}/{o}")
    for fn in sorted(set(fa) | set(fb)):
        if fa.get(fn) == fb.get(fn):
            continue
        ga, gb = gp(fa.get(fn, [])), gp(fb.get(fn, []))
        gained = {s: gb[s] - ga.get(s, 0) for s in gb if gb[s] > ga.get(s, 0)}
        lost = {s: ga[s] - gb.get(s, 0) for s in ga if ga[s] > gb.get(s, 0)}
        n += 1
        if n <= 60:
            print(f"{o[:-2]}:{fn} len {len(fa.get(fn, []))}->{len(fb.get(fn, []))} gained {gained} lost {lost}")
print("differing functions:", n)
