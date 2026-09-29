#!/usr/bin/env python3
"""Map func_800187F4's user-variable pseudos from a dump dir (tmp/func_800187F4/dump_<tag>):
for every (set (reg/v:SI N) RHS) in f.i.lreg, the setting insns with a short RHS; the flow
'Register N used ...' line (f.i.lreg carries the flow stats), the local-alloc / global-alloc
disposition, and the instrumented ALLOCDBG line (alloc.txt from alloc2.sh).
Usage: python3 pmap.py <dumpdir> [pseudo ...]   (no pseudo list = every reg/v pseudo)"""
import re
import sys

d = sys.argv[1]
want = set(sys.argv[2:])
lreg = open(d + "/f.i.lreg").read()
greg = open(d + "/f.i.greg").read()
alloc = open(d + "/alloc.txt").read()


def func_body(text):
    m = re.search(r"^;; Function func_800187F4\n(.*?)(?=^;; Function |\Z)", text, re.S | re.M)
    return m.group(1)


lb, gb = func_body(lreg), func_body(greg)
sets = {}
for blk in re.split(r"\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )", lb):
    m = re.match(r"\((\w+) (\d+) ", blk)
    if not m:
        continue
    s = re.search(r"\(set \(reg/v:SI (\d+)\)\s*(.*)", blk, re.S)
    if not s:
        continue
    rhs = " ".join(s.group(2).split())
    rhs = re.sub(r"\s*\(expr_list.*", "", rhs)
    sets.setdefault(s.group(1), []).append((int(m.group(2)), rhs[:110]))


def disp(body, r):
    m = re.search(r";; Register %s in (-?\d+)\." % r, body)
    return m.group(1) if m else None


gdisp = gb.split(";; Register dispositions:")[-1] if ";; Register dispositions:" in gb else ""
for r in sorted(sets, key=int):
    if want and r not in want:
        continue
    fl = re.search(r"^Register %s used .*$" % r, lb, re.M)
    g = re.search(r"(?:^|\s)%s in (\d+)" % r, gdisp)
    a = re.search(r"ALLOCDBG func=func_800187F4 ord=(\d+) pseudo=%s hardreg=(-?\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)" % r, alloc)
    loc = disp(lb, r)
    print(f"pseudo {r}: " + ("local-alloc -> %s" % loc if loc else "global ->  %s" % (g.group(1) if g else "?")))
    if fl:
        print("   " + fl.group(0)[:160])
    if a:
        print(f"   ALLOCDBG ord={a.group(1)} hardreg={a.group(2)} nrefs={a.group(3)} livelen={a.group(4)} pri={a.group(5)}")
    for uid, rhs in sets[r][:6]:
        print(f"   set@{uid}: {rhs}")
    if len(sets[r]) > 6:
        print(f"   ... {len(sets[r])} sets")
