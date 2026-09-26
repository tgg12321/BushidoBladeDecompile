"""One row per dump tag: selector pseudo / .lreg summary / hard reg, if-arm entry pseudo / .lreg
summary / hard reg, and the compiled if-arm `addu` and else-arm `andi` (instrumented cc1 .s).
usage: python3 table.py <tag>...
"""
import os
import re
import sys

D = os.path.join(os.path.dirname(os.path.abspath(__file__)), "rtl")
N = {2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 16: "s0", 17: "s1", 18: "s2", 19: "s3", 20: "s4",
     21: "s5", 22: "s6", 23: "s7", 30: "fp", -1: "mem"}
HR = {"$%d" % k: v for k, v in N.items()}


def split(txt):
    return re.split(r"\n\n", txt)


for tag in sys.argv[1:]:
    lreg = open(os.path.join(D, tag + ".lreg")).read()
    greg = open(os.path.join(D, tag + ".greg")).read()
    disp = {int(a): int(b) for a, b in re.findall(r"(\d+) in (-?\d+)", greg[greg.index(";; Register dispositions"):])}
    ins = split(lreg)
    sel = ent = None
    for i, b in enumerate(ins):
        m = re.match(r"\(insn (\d+) [^\n]*\(set \((reg/v|reg):SI (\d+)\)\s+\(and:SI \(reg:SI \d+\)\s+\(const_int 1\)\)", b)
        if m and i and re.search(r"lshiftrt:SI \(subreg:SI \(reg:QI \d+\) 0\)\s+\(const_int 1\)", ins[i - 1]):
            sel = (m.group(1), m.group(2), int(m.group(3)))
        m = re.match(r"\(insn (\d+) [^\n]*\(set \((reg/v|reg):SI (\d+)\)\s+\(plus:SI \(reg:SI (\d+)\)\s+\(reg:SI (\d+)\)\)", b)
        if m and ent is None and re.search(r"\(set \(reg:SI %s\)\s+\(mem:SI \(symbol_ref:SI \(\"D_80102764\"\)" % m.group(4), lreg):
            ent = (m.group(1), m.group(2), int(m.group(3)), int(m.group(4)), int(m.group(5)))
    s = open(os.path.join(D, tag + ".fn.s")).read().splitlines()
    k = next(i for i, l in enumerate(s) if "D_80102764" in l)
    addu = next((l.strip() for l in s[k:k + 8] if l.strip().startswith("addu")), "?")
    k = next(i for i, l in enumerate(s) if "D_801027B0+4" in l)
    andi = next((l.strip() for l in reversed(s[max(0, k - 12):k]) if l.strip().startswith("andi")), "?")

    def summ(r):
        m = re.search(r"^Register %d used (.*)$" % r, lreg, re.M)
        return m.group(1) if m else "-"

    out = [tag]
    if sel:
        out.append("sel %s(%s) -> %s [%s]" % (sel[2], sel[1], N.get(disp.get(sel[2]), disp.get(sel[2])), summ(sel[2])))
    if ent:
        out.append("entry %s(%s) -> %s [%s]" % (ent[2], ent[1], N.get(disp.get(ent[2]), disp.get(ent[2])), summ(ent[2])))
    out.append("if-arm: %s | else-arm: %s" % (addu, andi))
    print("\n    ".join(out))
