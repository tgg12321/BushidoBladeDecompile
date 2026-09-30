"""One-variable-per-value respellings of func_8002D780's cross products (from r11/pv_all.c).

swap_<t>: in the tests listed, the cross_point statement is placed before the cross_center one.
si_<mask>: the listed values are written as a Ruling 4 compound-assignment split
  (`x = a * b; x -= c * d;`) instead of one statement; mask digits: c1 c2 c3 p1 p2 p3 (1 = split).
Writes tmp/func_8002D780/r11pv/*.c and the list to r11pv_list.txt."""
import itertools
import os

SRC = "tmp/func_8002D780/r11/pv_all.c"
OUT = "tmp/func_8002D780/r11pv"
os.makedirs(OUT, exist_ok=True)
base = open(SRC, encoding="utf-8").read()

I1, I2, I3 = " " * 8, " " * 12, " " * 16
W = {
    ("c", 1): (I1 + "s32 cross_center1 = ", "z0 * cx", "x0 * cz"),
    ("p", 1): (I1 + "s32 cross_point1 = ", "z0 * px", "x0 * pz"),
    ("c", 2): (I2 + "cross_center2 = ", "z2 * cx", "x2 * cz"),
    ("p", 2): (I2 + "cross_point2 = ", "z2 * px", "x2 * pz"),
    ("c", 3): (I3 + "cross_center3 = ", "(flag * ax)", "(dx * az)"),
    ("p", 3): (I3 + "cross_point3 = ", "(flag * (px - x0))", "(dx * bz)"),
}


def line(k):
    head, a, b = W[k]
    return f"{head}{a} - {b};"


for k in W:
    assert base.count(line(k)) == 1, line(k)


def swap(text, t):
    lc, lp = line(("c", t)), line(("p", t))
    ls = text.split("\n")
    ic, ip = ls.index(lc), ls.index(lp)
    assert ic < ip
    if t == 1:  # adjacent declarations
        ls[ic], ls[ip] = ls[ip], ls[ic]
    else:  # move the point statement to just before the center statement
        p = ls.pop(ip)
        ls.insert(ic, p)
    return "\n".join(ls)


def splitinit(text, k):
    head, a, b = W[k]
    var = head.strip().split()[-2]
    ind = head[:len(head) - len(head.lstrip())]
    if head.strip().startswith("s32 "):
        new = f"{head}{a};\n{ind}{var} -= {b};"
    else:
        new = f"{head}{a};\n{ind}{var} -= {b};"
    return text.replace(line(k), new)


paths = []
for n in range(1, 8):
    ts = [t for t in (1, 2, 3) if n >> (t - 1) & 1]
    t = base
    for x in ts:
        t = swap(t, x)
    p = f"{OUT}/swap_{''.join(map(str, ts))}.c"
    open(p, "w", encoding="utf-8", newline="\n").write(t)
    paths.append(p)
keys = [("c", 1), ("c", 2), ("c", 3), ("p", 1), ("p", 2), ("p", 3)]
for bits in itertools.product((0, 1), repeat=6):
    if not any(bits):
        continue
    t = base
    if bits[0] or bits[3]:
        # C89: test 1's values become declared-then-assigned so a split can follow a statement
        t = t.replace(line(("c", 1)) + "\n" + line(("p", 1)),
                      f"{I1}s32 cross_center1;\n{I1}s32 cross_point1;\n\n"
                      f"{I1}cross_center1 = z0 * cx - x0 * cz;\n{I1}cross_point1 = z0 * px - x0 * pz;")
        W[("c", 1)] = (I1 + "cross_center1 = ", "z0 * cx", "x0 * cz")
        W[("p", 1)] = (I1 + "cross_point1 = ", "z0 * px", "x0 * pz")
    for b, k in zip(bits, keys):
        if b:
            t = splitinit(t, k)
    W[("c", 1)] = (I1 + "s32 cross_center1 = ", "z0 * cx", "x0 * cz")
    W[("p", 1)] = (I1 + "s32 cross_point1 = ", "z0 * px", "x0 * pz")
    p = f"{OUT}/si_{''.join(map(str, bits))}.c"
    open(p, "w", encoding="utf-8", newline="\n").write(t)
    paths.append(p)
t = base.replace(line(("c", 1)) + "\n" + line(("p", 1)),
                 f"{I1}s32 cross_center1;\n{I1}s32 cross_point1;\n\n"
                 f"{I1}cross_center1 = z0 * cx - x0 * cz;\n{I1}cross_point1 = z0 * px - x0 * pz;")
assert t != base
p = f"{OUT}/decl_assign.c"
open(p, "w", encoding="utf-8", newline="\n").write(t)
paths.append(p)
open("tmp/func_8002D780/r11pv_list.txt", "w", encoding="utf-8", newline="\n").write(",".join(paths))
print(len(paths))
