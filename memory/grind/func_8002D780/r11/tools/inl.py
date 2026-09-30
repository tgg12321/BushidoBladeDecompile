"""Variants from r11/pv_all.c with test 1's centroid cross product written inline in the `if`.
pinl_dead.c: the permuter's find (cross_center1 still declared, never read).
pinl.c: no cross_center1 at all.
pinl_both.c: both test-1 cross products inline (no test-1 locals).
pinl_p.c: test-1 point cross product inline, center kept as cross_center1."""
src = open("tmp/func_8002D780/r11/pv_all.c", encoding="utf-8").read()
C = "        s32 cross_center1 = z0 * cx - x0 * cz;\n"
P = "        s32 cross_point1 = z0 * px - x0 * pz;\n"
IF = "        if ((cross_center1 ^ cross_point1) >= 0) {\n"
for s in (C, P, IF):
    assert src.count(s) == 1
EC, EP = "(z0 * cx - x0 * cz)", "(z0 * px - x0 * pz)"


def w(name, t):
    open(f"tmp/func_8002D780/{name}.c", "w", encoding="utf-8", newline="\n").write(t)


w("pinl_dead", src.replace(IF, IF.replace("cross_center1", EC)))
w("pinl", src.replace(C, "").replace(IF, IF.replace("cross_center1", EC)))
w("pinl_both", src.replace(C, "").replace(P, "").replace(IF, f"        if (({EC} ^ {EP}) >= 0) {{\n"))
w("pinl_p", src.replace(P, "").replace(IF, IF.replace("cross_point1", EP)))
print("ok")
