"""func_8001F2E4 cleanup variants: split each reused local / drop the tgt_x constant-holder.
Each variant = the landed body with only the named variables renamed per block (+ declarations)."""
import os, re, itertools
ROOT = "/mnt/c/Users/Trenton/Desktop/Bushido Blade 2 Decompile"
# the landed body (banked at ../../rejected/retro-audit-2026-09-29.c); variants go to ../variants/
HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "..", "variants")
body = open(os.path.join(HERE, "..", "..", "rejected", "retro-audit-2026-09-29.c")).read()

M3 = "    t = *(s16 *)(obj + 0xC);\n"
M4 = "    if ((u32)(*(u16 *)(obj + 0xE) - 6) < 2U"
M5 = "    if (*(s16 *)(obj + 0x26E) != 0"
i3, i4, i5 = body.index(M3), body.index(M4), body.index(M5)
DECL_END = body.index("    s16 t;\n") + len("    s16 t;\n")


def split(b, var, new, seg):
    """rename var -> new inside segment seg ('b3','b4','b5','b4delta'); returns new body"""
    j3, j4, j5 = b.index(M3), b.index(M4), b.index(M5)
    lo, hi = {"b3": (j3, j4), "b4": (j4, j5), "b5": (j5, len(b))}[seg]
    part = re.sub(r"\b%s\b" % var, new, b[lo:hi])
    b = b[:lo] + part + b[hi:]
    d = b.index("    s16 t;\n")
    return b[:d] + "    s32 %s;\n" % new + b[d:]


def split_b4delta(b):
    j4, j5 = b.index(M4), b.index(M5)
    k = re.compile(r"        ang = \(\w+ - \*\(s16 \*\)\(obj \+ 0x1EA\)\) & 0xFFF;").search(b, j4).start()
    part = re.sub(r"\bang\b", "delta", b[k:j5])
    b = b[:k] + part + b[j5:]
    d = b.index("    s16 t;\n")
    return b[:d] + "    s32 delta;\n" + b[d:]


def no_tgtx(b):
    b = b.replace("    s32 tgt_x;\n", "")
    b = re.sub(r"\n *tgt_x = 0;", "", b)
    return b.replace("*(s16 *)(obj + 0x1E8), tgt_x);", "*(s16 *)(obj + 0x1E8), 0);")


OPS = {
    "twist3": lambda b: split(b, "ang", "twist", "b3"),
    "jitter5": lambda b: split(b, "ang", "jitter", "b5"),
    "tgt4": lambda b: split(b, "tgt_y", "twist_tgt", "b4"),
    "delta4": split_b4delta,
    "dist4": lambda b: split(split(split(split(b, "dx", "dx2", "b4"), "dz", "dz2", "b4"), "dist_sq", "dist_sq2", "b4"), "dist", "dist2", "b4"),
    "tgtx0": no_tgtx,
}
V = {"base": body}
for n, f in OPS.items():
    V["only_" + n] = f(body)
allb = body
for n in ["twist3", "jitter5", "tgt4", "delta4", "dist4"]:
    allb = OPS[n](allb)
V["split_all"] = allb
V["split_all_tgtx0"] = no_tgtx(allb)
for n, f in OPS.items():
    if n == "tgtx0":
        continue
    b = body
    for m in ["twist3", "jitter5", "tgt4", "delta4", "dist4"]:
        if m != n:
            b = OPS[m](b)
    V["keep_" + n] = b  # everything split except n
for n, b in V.items():
    open(f"{OUT}/{n}.c", "w").write(b)
print("ok", len(V))
for var in ["dx", "dz", "dist_sq", "dist"]:
    V2 = split(body, var, var + "2", "b4")
    open(f"{OUT}/only_{var}_b4.c", "w").write(V2)
print("ok singles")
b = split(split(body, "dist_sq", "dist_sq2", "b4"), "dist", "dist2", "b4")
open(f"{OUT}/only_dsq_dist_b4.c", "w").write(b)
open(f"{OUT}/only_dx_dz_b4.c", "w").write(split(split(body, "dx", "dx2", "b4"), "dz", "dz2", "b4"))
print("ok combos")
# tgt_x alternatives
b = re.sub(r"\n *tgt_x = 0;", "", body).replace("    s32 tgt_x;\n", "    s32 tgt_x = 0;\n")
open(f"{OUT}/tgtx_init_once.c", "w").write(b)
b = re.sub(r"\n *tgt_x = 0;", "", body)
b = b.replace("    ang = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;", "    tgt_x = 0;\n    ang = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;", 1)
open(f"{OUT}/tgtx_after_join.c", "w").write(b)
b = re.sub(r"\n *tgt_x = 0;", "", body)
b = b.replace("    func_8002F770((s16 *)(a + 0x36),", "    tgt_x = 0;\n    func_8002F770((s16 *)(a + 0x36),", 1)
open(f"{OUT}/tgtx_before_call.c", "w").write(b)
print("ok tgtx")
# ang value ablations for the first two easing deltas and each jitter
J1 = "    ang = (tgt_y - *(s16 *)(obj + 0x1E8)) & 0xFFF;"
k1, k2 = body.index("    ang = (tgt_z - *(s16 *)(obj + 0x1E6)) & 0xFFF;"), body.index(J1)
k3 = body.index("    func_8002F770((s16 *)(a + 0x36),")
def rename_range(b, lo, hi, old, new):
    part = re.sub(r"\b%s\b" % old, new, b[lo:hi])
    b = b[:lo] + part + b[hi:]
    d = b.index("    s16 t;\n")
    return b[:d] + "    s32 %s;\n" % new + b[d:]
open(f"{OUT}/only_d1E6.c", "w").write(rename_range(body, k1, k2, "ang", "d1e6"))
open(f"{OUT}/only_d1E8.c", "w").write(rename_range(body, k2, k3, "ang", "d1e8"))
j5 = body.index(M5)
jj = body.index("        ang = (rng_Next() & 0x3F) - 0x20;\n        *(u16 *)(a + 0x1E)")
open(f"{OUT}/only_jit1.c", "w").write(rename_range(body, j5, jj, "ang", "jit1"))
open(f"{OUT}/only_jit2.c", "w").write(rename_range(body, jj, len(body), "ang", "jit2"))
print("ok ang ablations")
