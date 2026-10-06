#!/usr/bin/env python3
# F13, on top of f03b (committed, 110ccc84e): the POLY_FT4 views in func_8006295C / func_80063084 / func_80063E10.
# Word stores into a POLY_FT4 (`*(s32 *)&prim->r0`, `->x0..x3`, `->u0 / u1`, `*(u16 *)&prim->u2 / u3`)
# and the tag word (`*(u32 *)prim`). Typed forms are measured per cluster (opt=<cluster>); a cluster
# whose typed form moves bytes stays as at HEAD (debt row).
# usage: f13.py [measure [func...]] [opt=<name>,...]   writes tmp/p2/f13/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f03b as B
sys.argv = _argv
OUT = "tmp/p2/f13/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = B.sub1
fn = B.fn

def b63084(b):
    if "tag" in OPT:
        b = sub1(b, "*(u32 *)prim = (*(u32 *)prim & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);",
                 "prim->tag = (prim->tag & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);")
    if "tag" not in OPT:   # default: through D_800A34E8 (= &prim->tag, set on the line before), as the siblings
        b = sub1(b, "*(u32 *)prim = (*(u32 *)prim & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);",
                 "*D_800A34E8 = (*D_800A34E8 & 0xFF000000) | (*(u32 *)D_800A34E4 & 0xFFFFFF);")
    if "rgb" in OPT:
        b = sub1(b, "*(s32 *)&prim->r0 = 0x808080;", "setRGB0(prim, 0x80, 0x80, 0x80);")
    if "xy" in OPT:
        b = sub1(b, "*(s32 *)&prim->x0 = *D_800A34B8 + *D_800A34BC - (*D_800A34C4 << 16);",
                 "prim->x0 = *D_800A34B8 + *D_800A34BC;\n                    prim->y0 = (*D_800A34B8 + *D_800A34BC - (*D_800A34C4 << 16)) >> 16;")
    if "uv0" in OPT:
        b = sub1(b, "*(s32 *)&prim->u0 = *D_800A34D4 + *D_800A3494;",
                 "prim->u0 = *D_800A34D4 + *D_800A3494;\n                    prim->v0 = (*D_800A34D4 + *D_800A3494) >> 8;\n"
                 "                    prim->clut = (*D_800A34D4 + *D_800A3494) >> 16;")
    if "uv" in OPT:
        b = sub1(b, "*(u16 *)&prim->u2 = *D_800A34DC;", "prim->u2 = *D_800A34DC;\n                    prim->v2 = *D_800A34DC >> 8;")
    return b

def b63E10(b):
    if "xy" in OPT:
        b = sub1(b, "*(s32 *)&prim->x0 = sxy[0];", "prim->x0 = sxy[0];\n        prim->y0 = sxy[0] >> 16;")
    return b

BODIES = [("func_80063084", b63084), ("func_80063E10", b63E10)]

BASE_REV = "110ccc84e"   # f03b as committed (equal to f03b.py's output)

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    s, g, h, t = (show("src/main/51268.c"), show("include/game.h"), show("include/bb2.h"),
                  show("src/main/64FD8.c"))
    for f, g_ in BODIES:
        s = fn(s, f, g_)
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

if __name__ == "__main__":
    s, g, h, t = write()
    if "measure" in sys.argv[1:]:
        fs = [a for a in sys.argv[1:] if a.startswith("func_")] or [f for f, _ in BODIES]
        B.A.Q.D2.D1.measure(s, g, h, t, fs)
    print("wrote f13", sorted(OPT))
