#!/usr/bin/env python3
"""mkvariants.py: write the full-build exclude-file variants to /tmp/q56/variants/."""
import os, re
T = "/tmp/q56/tree"
V = "/tmp/q56/variants"
os.makedirs(V, exist_ok=True)
lines = open(T + "/sdata_exclude.txt").read().split("\n")
DEAD = [18,19,20,34,40,43,44,45,47,48,54,57,59,66,70,73,74,76,87,90,92,95,101,102,103,104,105,106,107,108,109]
TRIM = {5: ["D_800A3770"], 6: ["D_800A3770"], 24: ["D_800A3930"], 41: ["D_800A344C", "D_800A3454"],
        55: ["D_800A3518"], 61: ["D_800A3518"], 64: ["D_800A3518"], 67: ["D_800A3518"], 71: ["D_800A3518"],
        78: ["D_800A3588", "D_800A358C"]}
def write(name, keep):
    open(f"{V}/{name}", "w", newline="\n").write("\n".join(keep))
hdr = [l for l in lines if l.strip().startswith("#") or not l.strip()]
write("all_removed.txt", [l for i, l in enumerate(lines, 1) if l.strip().startswith("#") or not l.strip()])
cleaned = [l for i, l in enumerate(lines, 1) if i not in DEAD]
write("cleaned.txt", cleaned)
trim = []
for i, l in enumerate(lines, 1):
    if i in DEAD:
        continue
    if i in TRIM:
        f, syms = l.split(":", 1)
        ks = [s.strip() for s in syms.split(",") if s.strip() and s.strip() not in TRIM[i]]
        l = f + ": " + ", ".join(ks)
    trim.append(l)
write("cleaned_trim.txt", trim)
write("r5.txt", [l for i, l in enumerate(lines, 1) if i != 5])
write("r18.txt", [l for i, l in enumerate(lines, 1) if i != 18])
open("/tmp/q56/dead_linenos.txt", "w").write("\n".join(map(str, DEAD)) + "\n")
print("ok", len(cleaned), len(trim))
