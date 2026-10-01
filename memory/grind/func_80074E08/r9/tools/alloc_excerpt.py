"""Excerpt the four `+ 0xC` sets, their pseudos' lifetimes and seats from r9/tools/dumps_all.sh output.
usage: python alloc_excerpt.py <outroot>  (reads <outroot>/fd_{reuse,pv_fn,nolocal}/func_80074E08.{lreg,greg})"""
import re, sys
root = sys.argv[1]
print("# func_80074E08 allocation dump excerpts (2026-10-01), r9/tools/dumps_all.sh on r9/variants/{reuse,pv_fn,nolocal}.c")
print("# cc1: tools/gcc-2.7.2/build/cc1 -O2 -G0 -funsigned-char -quiet -mcpu=3000 -mips1 -mno-abicalls -fno-builtin -w -mel -msoft-float -dl -dg -df")
for v in ("reuse", "pv_fn", "nolocal"):
    lreg = open(f"{root}/fd_{v}/func_80074E08.lreg").read()
    greg = open(f"{root}/fd_{v}/func_80074E08.greg").read()
    sets = re.findall(r"\(insn (\d+) \d+ \d+ \(set \(reg(?:/v)?:SI (\d+)\)\s+\(plus:SI \(reg:SI \d+\)\s+\(const_int 12\)\)\) 3 \{addsi3_internal\}", lreg)[:4]
    disp = dict(re.findall(r"(\d+) in (\d+)", greg[greg.index("Register dispositions"):]))
    print(f"\n== {v}: the four '+ 0xC' sets")
    for insn, r in sets:
        life = re.search(r"^Register %s used.*$" % r, lreg, re.M).group(0)
        loc = re.search(r";; Register %s in (\d+)" % r, lreg)
        seat = ("local-alloc " + loc.group(1)) if loc else ("global " + disp.get(r, "?"))
        print(f"   insn {insn}: pseudo {r}, {seat} | {life}")
