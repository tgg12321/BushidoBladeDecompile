"""For a dump tag: every same-side xor insn in .lreg -> (xor out pseudo, in1, in2),
the .lreg 'Register N used ...' line of each input, and the final hard regs
from .greg's 'Register dispositions' (pseudos local-alloc took are listed there too).
usage: python3 xormap.py <tag>"""
import re
import sys

tag = sys.argv[1]
d = "tmp/func_8002DE20/rtl/"
lreg = open(d + tag + ".lreg").read()
greg = open(d + tag + ".greg").read()
regline = {int(m.group(1)): m.group(0) for m in
           re.finditer(r"^Register (\d+) used [^\n]*", lreg, re.M)}
disp = {}
m = re.search(r";; Register dispositions:\n(.*?)\n\n", greg, re.S)
for a, b in re.findall(r"(\d+) in (\d+)", m.group(1)):
    disp[int(a)] = int(b)
NAMES = {2: "v0", 3: "v1", 4: "a0", 5: "a1", 6: "a2", 7: "a3", 8: "t0", 9: "t1",
         10: "t2", 11: "t3", 12: "t4", 13: "t5", 14: "t6", 15: "t7", 16: "s0",
         24: "t8", 25: "t9"}
xors = re.findall(r"\(insn (\d+) \d+ \d+ \(set \(reg:SI (\d+)\)\n\s+\(xor:SI \(reg(?:/v)?:SI (\d+)\)\n\s+\(reg(?:/v)?:SI (\d+)\)\)\)", lreg)
print(f"== {tag}: {len(xors)} xor insns (insn, out, in1, in2 -> hard regs)")
for k, (insn, out, i1, i2) in enumerate(xors, 1):
    o, a, b = int(out), int(i1), int(i2)
    h = lambda r: NAMES.get(disp.get(r, -1), str(disp.get(r)))
    tie = " TIED" if disp.get(o) in (disp.get(a), disp.get(b)) else ""
    print(f"test {k:2d}: insn {insn}: xor {o}({h(o)}) <- {a}({h(a)}) ^ {b}({h(b)}){tie}")
seen = set()
for insn, out, i1, i2 in xors:
    for r in (int(i1), int(i2)):
        if r not in seen:
            seen.add(r)
            print("   ", regline.get(r, f"Register {r}: (no line)"))
