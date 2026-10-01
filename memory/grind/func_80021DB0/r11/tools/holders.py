#!/usr/bin/env python3
"""holders.py DUMPDIR PSEUDO HARDREG : which pseudos conflicting with PSEUDO were seated in HARDREG (.greg)."""
import re, sys
d, p, hr = sys.argv[1], int(sys.argv[2]), int(sys.argv[3])
g = open(f"{d}/tu.i.greg.fn").read()
m = re.search(rf"^;; {p} conflicts:(.*)$", g, re.M)
conf = [int(x) for x in m.group(1).split()] if m else []
disp = {int(a): int(b) for a, b in re.findall(r"(\d+) in (\d+)", g.split("Register dispositions:")[1])}
alloc = {}
for ln in open(f"{d}/alloc.txt"):
    am = re.search(r"ord=(\d+) pseudo=(\d+) hardreg=(-?\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)", ln)
    if am:
        alloc[int(am.group(2))] = am.group(0)
for c in conf:
    if c >= 64 and disp.get(c) == hr:
        print(c, alloc.get(c, "local"))
