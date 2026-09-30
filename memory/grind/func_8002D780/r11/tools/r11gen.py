"""Ruling 11 variants for func_8002D780's cross_center / cross_point (was kc / kp).

Base = memory/grind/func_8002D780/candidate.c. Writes tmp/func_8002D780/r11/*.c.
Values: 1 = edge v0-v1 (test 1, outer block), 2 = edge v0-v2 (test 2), 3 = edge v1-v2 (test 3).
Each split gives a value its own fresh local, declared at the innermost scope enclosing its
writes (test 1: the outer block; test 2: the `if` arm of test 1; test 3: the `if` arm of test 2).
A shared variable whose value 1 is split out is declared (uninitialized) at the top of the
test-1 arm, the innermost scope enclosing its remaining writes.
"""
import os
import re

BASE = "memory/grind/func_8002D780/candidate.c"
OUT = "tmp/func_8002D780/r11"
os.makedirs(OUT, exist_ok=True)
src = open(BASE, encoding="utf-8").read()

ren = src
for a, b in (("s32 kc =", "s32 cross_center ="), ("s32 kp =", "s32 cross_point ="),
             ("kc = ", "cross_center = "), ("kp = ", "cross_point = "),
             ("(kc ^ kp)", "(cross_center ^ cross_point)")):
    ren = ren.replace(a, b)
for tok in ("kc ", "kp ", "(kc", "kp)"):
    assert tok not in ren, tok

NAMES = {"c": "cross_center", "p": "cross_point"}
IND = {1: " " * 8, 2: " " * 12, 3: " " * 16}
RHS = {("c", 1): "z0 * cx - x0 * cz;", ("p", 1): "z0 * px - x0 * pz;",
       ("c", 2): "z2 * cx - x2 * cz;", ("p", 2): "z2 * px - x2 * pz;",
       ("c", 3): "(flag * ax) - (dx * az);", ("p", 3): "(flag * (px - x0)) - (dx * bz);"}


def lines_of(text):
    return text.split("\n")


def find(ls, pred):
    hits = [i for i, l in enumerate(ls) if pred(l)]
    assert len(hits) == 1, hits
    return hits[0]


def split(text, var, val):
    name = NAMES[var]
    new = f"{name}{val}"
    ls = lines_of(text)
    rhs = RHS[(var, val)]
    ind = IND[val]
    w = find(ls, lambda l: l.startswith(ind) and not l.startswith(ind + " ") and l.endswith(rhs))
    ifl = find(ls, lambda l: l.startswith(ind + "if ((cross_") and not l.startswith(ind + " "))
    if val == 1:
        assert ls[w] == f"{ind}s32 {name} = {rhs}", ls[w]
        ls[w] = f"{ind}s32 {new} = {rhs}"
        ls[ifl] = re.sub(rf"\b{name}\b", new, ls[ifl])
        # shared variable keeps values 2/3: declare it at the top of the test-1 arm
        ls.insert(ifl + 1, f"{IND[2]}s32 {name};")
    elif val == 2:
        assert ls[w] == f"{ind}{name} = {rhs}", ls[w]
        ls[w] = f"{ind}{new} = {rhs}"
        ls[ifl] = re.sub(rf"\b{name}\b", new, ls[ifl])
        # declared at the top of the test-1 arm (C89: declarations before statements)
        t1 = find(ls, lambda l: l.startswith(IND[1] + "if ((cross_") and not l.startswith(IND[1] + " "))
        ls.insert(t1 + 1, f"{IND[2]}s32 {new};")
    else:
        assert ls[w] == f"{ind}{name} = {rhs}", ls[w]
        ls[w] = f"{ind}{new} = {rhs}"
        ls[ifl] = re.sub(rf"\b{name}\b", new, ls[ifl])
        d = find(ls, lambda l: l == f"{IND[3]}s32 bz;")
        ls.insert(d + 1, f"{IND[3]}s32 {new};")
    return "\n".join(ls)


def cleanup(text):
    """Drop a shared declaration whose variable is no longer written anywhere."""
    for name in NAMES.values():
        decl = f"{IND[2]}s32 {name};"
        if decl in text:
            rest = text.replace(decl + "\n", "")
            if not re.search(rf"\b{name}\b", rest):
                text = rest
    return text


def write(name, text):
    p = f"{OUT}/{name}.c"
    open(p, "w", encoding="utf-8", newline="\n").write(text)
    return p


out = [write("reuse_renamed", ren)]
pv = ren
for var in "cp":
    for val in (1, 2, 3):
        pv = split(pv, var, val)
out.append(write("pv_all", cleanup(pv)))
for var in "cp":
    for val in (1, 2, 3):
        out.append(write(f"abl_{var}{val}", cleanup(split(ren, var, val))))
for val in (1, 2, 3):
    out.append(write(f"pair_{val}", cleanup(split(split(ren, "c", val), "p", val))))
for var in "cp":
    t = ren
    for val in (1, 2, 3):
        t = split(t, var, val)
    out.append(write(f"var_{var}_all", cleanup(t)))
# minimal reuse: cross_center fully split; cross_point keeps value 1 plus value 2 and/or 3
csplit = ren
for val in (1, 2, 3):
    csplit = split(csplit, "c", val)
csplit = cleanup(csplit)
out.append(write("min_p123", csplit))
out.append(write("min_p12", cleanup(split(csplit, "p", 3))))
out.append(write("min_p13", cleanup(split(csplit, "p", 2))))
# one-variable-per-value with the test-2 values as initialized declarations (the first pv form)
pvi = pv.replace("            s32 cross_center2;\n", "").replace("            s32 cross_point2;\n", "")
pvi = pvi.replace("            cross_center2 = z2", "            s32 cross_center2 = z2")
pvi = pvi.replace("            cross_point2 = z2", "            s32 cross_point2 = z2")
out.append(write("pv_all_init", cleanup(pvi)))
print(",".join(out))
