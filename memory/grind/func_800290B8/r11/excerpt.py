#!/usr/bin/env python3
"""Excerpt the allocator facts for func_800290B8 from d_<v>/f.{lreg,greg}: every multi-block
pseudo's lreg summary line, its greg disposition, and the RTL insns computing i/2 and i&1."""
import re, sys

for v in sys.argv[1:]:
    lreg = open(f"d_{v}/f.lreg").read()
    greg = open(f"d_{v}/f.greg").read()
    disp = dict(re.findall(r"(\d+) in (\d+)", greg[greg.index("dispositions"):]))
    print(f"=== {v}")
    for line in lreg.splitlines():
        if line.startswith("Register ") and "in block" not in line:
            r = line.split()[1]
            print(f"  {line}  ->  hard reg {disp.get(r, 'NONE (stack)')}")
    # the (ashiftrt ... 1) and (and ... 1) insns of the bounding-box loop, from .greg
    for m in re.finditer(r"\(insn (\d+) [^\n]*\n\s+\(set \(reg:SI (\d+)[^\n]*\n?\s*\((ashiftrt|and):SI \(reg:SI (\d+)[^\n]*\n?[^\n]*\(const_int 1\)", greg):
        print(f"  insn {m.group(1)}: pseudo {m.group(2)} = ({m.group(3)} pseudo {m.group(4)}, 1)")
    for l in greg.splitlines():
        if l.startswith("Spilling"):
            print("  " + l)
