#!/usr/bin/env python3
"""mkall.py <reuse candidate.c> <outdir>
Derive the Ruling 11 comparison bodies for func_800290B8 from the reuse spelling:
  pv.c        one variable per value (row/col block-local in the bounding-box loop,
              mark at function scope, tri at marker-loop-body scope) -- same statements
  abl_a.c     only tmp_a split (row / mark), tmp_b still shared
  abl_b.c     only tmp_b split (col / tri), tmp_a still shared
  r_inline.c  pv with the corner index written inline (no row/col locals)
  r_while.c   pv with the marker loop as a while loop, steps at the end of the body
  r_index.c   pv with the markers read as list[mark].field instead of a walking pointer
  f_self.c    pv + SOTN self-assign `mark = mark;` in the triangle loop
  f_self1.c   pv + `mark = mark;` in the bounding-box loop
  f_dead.c    pv + a dead store `mark = row;` in the bounding-box loop
"""
import os, sys

src, out = sys.argv[1], sys.argv[2]
os.makedirs(out, exist_ok=True)
s = open(src).read()


def sub(text, old, new, count=1):
    assert text.count(old) >= 1, old
    return text.replace(old, new) if count == 0 else text.replace(old, new, count)


decl_old = s[s.index("    s32 i, vtx;\n"):s.index("    s32 tmp_b;\n") + len("    s32 tmp_b;\n")]
loop1_old = "        tmp_a = i / 2;\n        tmp_b = i & 1;\n        vtx = side * 4 + tmp_a * 2 + tmp_b;\n"

# --- one variable per value
pv = sub(s, decl_old, "    s32 i;\n    s32 mark;\n")
pv = sub(pv, loop1_old,
         "        s32 row = i / 2;\n        s32 col = i & 1;\n        s32 vtx = side * 4 + row * 2 + col;\n\n")
pv = sub(pv, "for (tmp_a = 0; e->type != 0; tmp_a++, e++) {\n",
         "for (mark = 0; e->type != 0; mark++, e++) {\n        s32 tri;\n\n")
pv = sub(pv, "func_80044B30(tmp_a,", "func_80044B30(mark,")
pv = pv.replace("tmp_b", "tri")
assert "tmp_" not in pv
open(out + "/pv.c", "w", newline="\n").write(pv)

# --- ablations (one variable split, the other still shared)
a = sub(s, loop1_old, "        row = i / 2;\n        tmp_b = i & 1;\n        vtx = side * 4 + row * 2 + tmp_b;\n")
a = sub(a, "    s32 i, vtx;\n", "    s32 i, vtx, row;\n")
open(out + "/abl_a.c", "w", newline="\n").write(a)
b = sub(s, loop1_old, "        tmp_a = i / 2;\n        col = i & 1;\n        vtx = side * 4 + tmp_a * 2 + col;\n")
b = sub(b, "    s32 i, vtx;\n", "    s32 i, vtx, col;\n")
open(out + "/abl_b.c", "w", newline="\n").write(b)

# --- structural respellings of the per-value body
r = sub(pv, "        s32 row = i / 2;\n        s32 col = i & 1;\n        s32 vtx = side * 4 + row * 2 + col;\n",
        "        s32 vtx = side * 4 + (i / 2) * 2 + (i & 1);\n")
open(out + "/r_inline.c", "w", newline="\n").write(r)

w = sub(pv, "for (mark = 0; e->type != 0; mark++, e++) {", "mark = 0;\n    while (e->type != 0) {")
w = w.replace(") continue;\n", ") goto next;\n")
w = sub(w, "        return 1;\n    }\n    return 0;\n}", "        return 1;\n    next:\n        mark++;\n        e++;\n    }\n    return 0;\n}")
open(out + "/r_while.c", "w", newline="\n").write(w)

x = sub(pv, "    e = (Marker_290B8 *)func_8004678C();\n", "    list = (Marker_290B8 *)func_8004678C();\n")
x = sub(x, "Marker_290B8 *e;", "Marker_290B8 *list;")
x = sub(x, "for (mark = 0; e->type != 0; mark++, e++) {", "for (mark = 0; list[mark].type != 0; mark++) {")
x = x.replace("e->", "list[mark].")
open(out + "/r_index.c", "w", newline="\n").write(x)

# --- sanctioned-family probes on the per-value body
t = "        for (tri = 0; tri < 2; tri++) {\n"
open(out + "/f_self.c", "w", newline="\n").write(sub(pv, t, t + "            mark = mark;\n"))
v = "        s32 vtx = side * 4 + row * 2 + col;\n"
open(out + "/f_self1.c", "w", newline="\n").write(sub(pv, v, v + "\n        mark = mark;\n"))
open(out + "/f_dead.c", "w", newline="\n").write(sub(pv, v, v + "\n        mark = row;\n"))
print("ok")
