"""edits.py <text1b.c> <code6cac.h> [--no-e84]: the func_80057E84 data-model edits, in place.
Run from the repo root (imports engine.inlineasm). Anchor-based, asserts every anchor once."""
import re
import sys
from pathlib import Path

sys.path.insert(0, ".")
from engine import inlineasm  # noqa: E402

D = Path(__file__).parent
src_p, hdr_p = Path(sys.argv[1]), Path(sys.argv[2])
no_e84 = "--no-e84" in sys.argv
no_c0 = "--no-c0" in sys.argv
no_cc8 = "--no-cc8" in sys.argv


def once(s, a, b):
    assert s.count(a) == 1, (a[:60], s.count(a))
    return s.replace(a, b)


# ---- header ----
h = hdr_p.read_text(encoding="utf-8")
h = once(h, "/* One CPU path waypoint (PracticeMenuRec.unk_364[]): func_800571C0 writes x/z from the\n",
         "/* One CPU path waypoint (CpuRoute.node[]): func_800571C0 writes x/z from the\n")
h = once(h, "} CpuWaypoint;\n", """} CpuWaypoint;

/* A CPU route: the polygon and vertex the walker stands at (func_80057ACC), the waypoint
 * count and the waypoints. PracticeMenuRec carries one at +0x360; func_80057E84 builds two
 * candidates of the same layout on its stack and appends the cheaper one. */
typedef struct CpuRoute {
    u8 poly;                       /* index into the stage's NavPolySet.polys */
    u8 vtx;                        /* vertex index in that polygon */
    u8 count;                      /* number of waypoints in node[] */
    CpuWaypoint node[8];
} CpuRoute;                        /* sizeof == 0x34 */

/* One polygon of a stage's navigation set (8 bytes): flags (0x80 = open chain, the last vertex
 * does not close back to the first), a kind byte copied into the route waypoints built around
 * it, a corner margin (func_80057CC8 scales it by 40), the vertex count and the vertex table of
 * x/z pairs. */
typedef struct NavPoly {
    u8 flags;
    u8 kind;
    u8 margin;
    u8 nvtx;
    s16 (*vtx)[2];
} NavPoly;

/* A stage's navigation set: a D_8009A658 row (12 bytes: count word, polygon array, zero word). */
typedef struct NavPolySet {
    u8 npolys;
    u8 unk1[3];
    NavPoly *polys;
    u8 unk8[4];
} NavPolySet;
""")
h = once(h, """    u8  unk_352[0x362 - 0x352];
    u8  unk_362;                   /* number of waypoints in unk_364 */
    u8  unk_363;
    CpuWaypoint unk_364[8];
""", """    u8  unk_352[0x360 - 0x352];
    CpuRoute cpu_route;            /* 0x360 */
""")
hdr_p.write_bytes(h.encode("utf-8"))

# ---- text1b.c ----
s = src_p.read_text(encoding="utf-8")
n362 = s.count("->unk_362")
n364 = s.count("->unk_364[")
s = s.replace("->unk_362", "->cpu_route.count").replace("->unk_364[", "->cpu_route.node[")
print(f"unk_362 x{n362}, unk_364 x{n364} respelled")
s = inlineasm.substitute_body(s, "func_80057ACC", (D / "acc.c").read_text())
if not no_cc8:
    s = inlineasm.substitute_body(s, "func_80057CC8", (D / __import__("os").environ.get("CC8", "cc8c.c")).read_text())
if not no_c0:
    s = inlineasm.substitute_body(s, "func_800571C0", (D / "c0.c").read_text())
    s = once(s, "func_800571C0((s32)p)", "func_800571C0(p)")
if not no_e84:
    s = inlineasm.substitute_body(s, "func_80057E84", (D.parent / "candidate.c").read_text())
s = once(s, " * vertex arg1 of the polygon whose vertex table hangs off arg0[4], and writes the\n",
         " * vertex arg1 of polygon arg0 (its vertex table arg0->vtx), and writes the\n")
s = once(s, " * FAKE: the vertex-table base expression *(s16 **)(arg0 + 4) is written out at each\n",
         " * FAKE: the vertex-table base expression arg0->vtx is written out at each\n")
s = once(s, " * call cannot write ((s16 **)arg0)[1], which C does not guarantee and which folds\n",
         " * call cannot write arg0->vtx, which C does not guarantee and which folds\n")
s = once(s, "extern u8 D_8009A658[][12];\n", "extern NavPolySet D_8009A658[];\n")
s = once(s, "extern void func_80057E84(PracticeMenuRec *, u8 *, s32, s32);\n",
         "extern void func_80057E84(PracticeMenuRec *, NavPolySet *, s32, s32);\n")
s = once(s, "    u8 *pois;\n", "    NavPolySet *pois;\n")
s = once(s, "pois = D_8009A658[D_800A36A4];", "pois = &D_8009A658[D_800A36A4];")
s = once(s, "func_80057ACC((s32)p, pois, tx, tz)", "func_80057ACC(p, pois, tx, tz)")
src_p.write_bytes(s.encode("utf-8"))
print("edits applied")
