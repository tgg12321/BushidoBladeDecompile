#!/usr/bin/env python3
"""structural.py <pv_all.c> <outdir> [reuse.c]: structural respellings of the full split body (pvs_*.c).
Each is pv_all.c (every value of vtx/node/route in its own local) with one structural change:
  pvs_noptr_vtx    no vertex pointers: poly->vtx[k][0] / [1] read directly at each site
  pvs_noptr_node   no waypoint pointers: route_X->node[c].x / .z / .kind stored directly
  pvs_noptr_route  no route pointers in the corner blocks: path[0] / path[1] directly
  pvs_noptr_all    the three above together (the tail keeps route_pick)
  pvs_tail_cond    the tail pick as one conditional expression
  pvs_node_early   node_X computed right after the count increment (before the c >= 7 test)
  pvs_vtx_struct   vertices as a struct {s16 x, z;} (vtx table typed NavVtx *)
  pvs_vtab         the vertex table loaded once per pass into a fresh `tab` (s16 (*)[2])
  pvs_copy_k       the copy loop with its own counter k (i is not reused; Q51/Q53 receipt)
  pvs_register     every split value declared egister (no asm)
"""
import re
import sys
from pathlib import Path

src = Path(sys.argv[1]).read_text()
out = Path(sys.argv[2])
out.mkdir(parents=True, exist_ok=True)


def sub(s, a, b, n=1):
    assert s.count(a) >= n, (a, s.count(a))
    return s.replace(a, b)


def noptr_vtx(s):
    for v, k in (("vtx_a", "i"), ("vtx_b", "next"), ("vtx_dn", "idx_dn"), ("vtx_up", "idx_up")):
        s = re.sub(r"\n\s*%s = poly->vtx\[%s\];" % (v, k), "", s)
        s = re.sub(r"\b%s\[([01])\]" % v, r"poly->vtx[%s][\1]" % k, s)
        s = re.sub(r"\n\s*s16 \*%s;" % v, "", s)
    return s


def noptr_node(s):
    for v, r in (("node_dn", "route_dn"), ("node_up", "route_up")):
        s = re.sub(r"\n\s*%s = &%s->node\[c\];" % (v, r), "", s)
        s = re.sub(r"\b%s->(x|z|kind)" % v, r"%s->node[c].\1" % r, s)
        s = re.sub(r"\n\s*CpuWaypoint \*%s;" % v, "", s)
    return s


def noptr_route(s):
    for v, k in (("route_dn", "0"), ("route_up", "1")):
        s = re.sub(r"\n\s*%s = &path\[%s\];" % (v, k), "", s)
        s = re.sub(r"\b%s->" % v, "path[%s]." % k, s)
        s = re.sub(r"\b%s(?=[^\w])" % v, "(&path[%s])" % k, s)
        s = re.sub(r"\n\s*RouteBuf \*\(&path\[%s\]\);" % k, "", s)
    return s


def tail_cond(s):
    return sub(s, "    if (dist_dn < dist_up) {\n        route_pick = &path[0];\n    } else {\n        route_pick = &path[1];\n    }\n",
               "    route_pick = dist_dn < dist_up ? &path[0] : &path[1];\n")


def node_early(s):
    for v, r, g in (("node_dn", "route_dn", "go_dn"), ("node_up", "route_up", "go_up")):
        line = "                %s = &%s->node[c];\n" % (v, r)
        s = sub(s, line, "")
        s = sub(s, "                %s->count = c + 1;\n" % r, "                %s->count = c + 1;\n%s" % (r, line))
    return s


def vtx_struct(s):
    s = sub(s, "typedef struct {\n    u8 flags;", "typedef struct {\n    s16 x;\n    s16 z;\n} NavVtx;\n\ntypedef struct {\n    u8 flags;")
    s = sub(s, "    s16 (*vtx)[2];\n} NavPoly;", "    NavVtx *vtx;\n} NavPoly;")
    for v in ("vtx_a", "vtx_b", "vtx_dn", "vtx_up"):
        s = re.sub(r"s16 \*%s;" % v, "NavVtx *%s;" % v, s)
        s = re.sub(r"%s = poly->vtx\[(\w+)\];" % v, r"%s = &poly->vtx[\1];" % v, s)
        s = s.replace("%s[0]" % v, "%s->x" % v).replace("%s[1]" % v, "%s->z" % v)
    return s


def vtab(s):
    s = sub(s, "        for (i = 0; i < nedges; i++) {\n", "        for (i = 0; i < nedges; i++) {\n            s16 (*tab)[2];\n\n            tab = poly->vtx;\n")
    s = s.replace("vtx_a = poly->vtx[i];", "vtx_a = tab[i];").replace("vtx_b = poly->vtx[next];", "vtx_b = tab[next];")
    return s


def register(s):
    return re.sub(r"(?m)^(\s*)(s16 \*|CpuWaypoint \*|RouteBuf \*)(vtx_|node_|route_)", r"\1register \2\3", s)


def copy_k(s):
    s = sub(s, "    s16 next;\n", "    s16 next;\n    s16 k;\n")
    return sub(s, "    for (i = route_pick->count - 1; i >= 0; i--) {\n        arg0->unk_364[arg0->unk_362] = route_pick->node[i];",
               "    for (k = route_pick->count - 1; k >= 0; k--) {\n        arg0->unk_364[arg0->unk_362] = route_pick->node[k];")


VARIANTS = {
    "pvs_noptr_vtx": [noptr_vtx], "pvs_noptr_node": [noptr_node], "pvs_noptr_route": [noptr_route],
    "pvs_noptr_all": [noptr_vtx, noptr_node, noptr_route], "pvs_tail_cond": [tail_cond],
    "pvs_node_early": [node_early], "pvs_vtx_struct": [vtx_struct], "pvs_vtab": [vtab],
    "pvs_copy_k": [copy_k], "pvs_register": [register],
}
for name, fns in VARIANTS.items():
    s = src
    for f in fns:
        s = f(s)
    (out / f"{name}.c").write_text(s)
if len(sys.argv) > 3:  # the reuse body with a fresh copy counter (the Q51 receipt for i)
    r = Path(sys.argv[3]).read_text().replace("route->", "route_pick->")
    r = copy_k(r).replace("route_pick->", "route->")
    (out / "q51_copy_k.c").write_text(r)
print(len(VARIANTS), "variants")
