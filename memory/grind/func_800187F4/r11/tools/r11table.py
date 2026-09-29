#!/usr/bin/env python3
"""Ruling 11 (D)(1) excerpt table for func_800187F4: for each dump dir, name the pseudos of the
seven reused locals (or their per-value splits) from their setting insns in f.i.lreg, and print
pseudo, allocator (local-alloc qty / global), hard register, and the instrumented ALLOCDBG line
(ord = global allocation order, nrefs, livelen, pri). Usage: r11table.py <dumpdir>..."""
import re
import sys

REG = {2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 7: "a3", 8: "t0", 9: "t1", 10: "t2", 11: "t3",
       12: "t4", 16: "s0", 17: "s1", 18: "s2", 19: "s3", 20: "s4", 21: "s5", 22: "s6", 23: "s7", 24: "t8", 30: "fp"}


def body(text):
    m = re.search(r"^;; Function func_800187F4\n(.*?)(?=^;; Function |\Z)", text, re.S | re.M)
    return m.group(1)


def classify(sets):
    """sets: list of RHS strings for one pseudo -> list of value labels."""
    lab = []
    for r in sets:
        if r.startswith("(const_int 0)") or re.match(r"\(plus:SI \(reg/v:SI \d+\) \(const_int 1\)\)", r):
            lab.append("counter")
        elif re.search(r"\(mem/s:SI \(plus:SI \(reg:SI \d+\) \(const_int (20|24|28|32|36|40)\)\)\)", r):
            k = int(re.search(r"const_int (\d+)\)\)\)", r).group(1))
            lab.append("node[%d]" % (k // 4 + 2))   # base = the node+8 giv
        elif re.match(r"\(reg/v:SI \d+\)", r):
            lab.append("copy")
        elif "D_8008D118" in r and "zero_extend" in r:
            lab.append("table byte")
        elif re.search(r"\(mem/s:SI \(plus:SI \(reg:SI 30 \$fp\) \(const_int 16\)\)\)", r):
            lab.append("lz[0]")
        elif re.search(r"\(mem/s:SI \(plus:SI \(reg:SI 30 \$fp\) \(const_int 20\)\)\)", r):
            lab.append("lz[1]")
        else:
            lab.append(r[:40])
    return lab


def run(d):
    lb = body(open(d + "/f.i.lreg").read())
    gb = body(open(d + "/f.i.greg").read())
    alloc = open(d + "/alloc.txt").read()
    sets = {}
    for blk in re.split(r"\n(?=\((?:insn|jump_insn|call_insn|code_label|note|barrier) )", lb):
        m = re.match(r"\((\w+) (\d+) ", blk)
        s = re.search(r"\(set \(reg/v:SI (\d+)\)\s*(.*)", blk, re.S) if m else None
        if s:
            sets.setdefault(int(s.group(1)), []).append((int(m.group(2)), " ".join(s.group(2).split())))
    gdisp = gb.split(";; Register dispositions:")[-1]
    print(f"== {d}")
    for p in sorted(sets):
        labs = classify([r for _u, r in sets[p]])
        rhs0 = sets[p][0][1]
        interesting = any(x in ("counter", "node[7]", "node[8]", "copy", "table byte", "lz[0]", "lz[1]") for x in labs) and not any(x in ("node[9]", "node[11]") for x in labs)
        # the ground / focus-0 Y deltas and the squared-length / root pseudos, by shape
        if not interesting:
            if re.match(r"\(minus:SI \(reg:SI \d+\) \(reg:SI \d+\)\)", rhs0) and len(sets[p]) <= 2:
                labs = ["minus"] * len(sets[p])
                interesting = True
            elif re.match(r"\(plus:SI \(reg:SI \d+\) \(reg:SI \d+\)\)", rhs0) and any("lshiftrt" in r for _u, r in sets[p]):
                labs = ["sum", "root..."]
                interesting = True
        if not interesting:
            continue
        loc = re.search(r";; Register %d in (-?\d+)\." % p, lb)
        g = re.search(r"(?:^|\s)%d in (\d+)" % p, gdisp)
        a = re.search(r"ord=(\d+) pseudo=%d hardreg=(-?\d+) nrefs=(\d+) livelen=(\d+) pri=(\d+)" % p, alloc)
        where = f"local-alloc {REG.get(int(loc.group(1)), loc.group(1))}" if loc else \
            f"global {REG.get(int(g.group(1)), g.group(1)) if g else '?'}"
        adbg = f"ord={a.group(1)} nrefs={a.group(3)} livelen={a.group(4)} pri={a.group(5)}" if a and not loc else ""
        uids = ",".join(str(u) for u, _r in sets[p])
        print(f"  pseudo {p:4d} {where:16s} {adbg:44s} sets@{uids}: {', '.join(labs)}")


for d in sys.argv[1:]:
    run(d)
