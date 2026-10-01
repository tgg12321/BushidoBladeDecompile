#!/usr/bin/env python3
"""initsyms.py: gp symbols in the initialized small-data region (< 0x800A3308): which C objects gp them,
where those objects sit in .text (link order), and whether the original bytes are zero."""
import json, re, subprocess
A = json.load(open("/tmp/q56/model_analysis.json"))
mp = open("/tmp/q56/tree/build/bb2.map").read()
pos = {}
for m in re.finditer(r"^\s*\.text\s+0x([0-9a-f]+)\s+0x([0-9a-f]+)\s+build/src/(\w+)\.o", mp, re.M):
    pos[m.group(3)] = (int(m.group(1), 16), int(m.group(2), 16))
multi, single = [], []
for s, v in sorted(A["symbols"].items(), key=lambda x: x[1]["addr"] or ""):
    a = int(v["addr"], 16)
    if a >= 0x800A3308:
        continue
    row = (s, hex(a), "nonzero" if v["orig_zero4"] is False else "zero", v["offset_gp"],
           [(o, hex(pos.get(o, (0, 0))[0]), hex(sum(pos.get(o, (0, 0))))) for o in v["objects"]])
    (multi if len(v["objects"]) > 1 else single).append(row)
print("initialized-region gp symbols:", len(multi) + len(single))
print("gp'd from ONE object:", len(single))
for r in single: print("  ", r)
print("gp'd from SEVERAL objects (those objects must be one original file):")
for r in multi: print("  ", r)
