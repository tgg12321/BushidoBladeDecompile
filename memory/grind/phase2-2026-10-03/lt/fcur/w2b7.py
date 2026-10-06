#!/usr/bin/env python3
# Worker-2 batch 7: the D_800A3820 draw-queue cursor family, plus Vec4i32 retired for PsyQ's VECTOR.
# D_800A3820 walks D_80102C00, a list of record pointers the asm consumers (3AB48 func_8004A4E0 /
# func_8004C404) read: transform nodes (Unk80101DF0Record and the records that embed it) and the
# 16-byte grid records of D_800A4750 (Unk800A4750Rec, a different layout), so the list holds void *.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/fcur/w2b7.py [opt=...] [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "84373e6f8"   # main (P7b landed as 6607ec419)
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, fn = B2.rep, B2.fn


def base(p):
    B2.BASE = BASE
    return B2.show(p)


def push(b, var, typ="void **"):
    """`T *var; var = (s32 *)D_800A3820; D_800A3820 = (s32)(var + 1); *var = (s32)X;` -> void ** cursor"""
    b, n1 = re.subn(r"\bs32 \*%s;" % var, "%s%s;" % (typ, var), b)
    b, n1b = re.subn(r"\bs32 \*%s = \(s32 \*\)D_800A3820;" % var, "%s%s = D_800A3820;" % (typ, var), b)
    b, n2 = re.subn(r"\b%s = \(s32 \*\)D_800A3820;" % var, "%s = D_800A3820;" % var, b)
    b, n3 = re.subn(r"D_800A3820 = \(s32\)\(%s \+ 1\);" % var, "D_800A3820 = %s + 1;" % var, b)
    b, n4 = re.subn(r"\*%s = \(s32\)(&?[\w.>-]+);" % var, r"*%s = \1;" % var, b)
    if not (n3 and n4 and (n1 or n1b)):
        raise SystemExit("push %s: %d %d %d %d %d" % (var, n1, n1b, n2, n3, n4))
    return b


RESET = ("D_800A3820 = (s32)&D_80102C00;", "D_800A3820 = &D_80102C00[0];" if "rs_idx" in OPT else
         "D_800A3820 = &D_80102C00;" if "rs_scalar" in OPT else "D_800A3820 = D_80102C00;")


def h_bb2(s):
    s = rep(s, "extern s32 D_800A3820;\n", "extern void **D_800A3820; /* the draw queue cursor (into D_80102C00) */\n")
    s = rep(s, "extern s32 D_80102C00;\n", "extern void *D_80102C00; /* the draw queue's first slot */\n" if "rs_scalar" in OPT else
            "extern void *D_80102C00[640]; /* the draw queue: record pointers (0x80102C00..0x801035FF) */\n" if "rs_unsized" not in OPT else
            "extern void *D_80102C00[]; /* the draw queue: record pointers */\n")
    s = s.replace("Vec4i32", "VECTOR")
    return s


VEC_NOTE_OLD = """/* Vec4i32 / SVec4i16 are the remaining local-name copies of the PsyQ VECTOR / SVECTOR
 * layouts (include/psxsdk/libgte.h); retyping them as the Sony types is Phase 2 work.  func_80022580 copies
 * Unk80101EC8Record's +0xB8 and +0x104 as whole 16-byte VECTORs (pad included)
 * and +0x1C8 as a whole 8-byte SVECTOR. */
typedef struct { s32 vx, vy, vz, pad; } Vec4i32;
"""
VEC_NOTE_NEW = """/* SVec4i16 is the remaining local-name copy of the PsyQ SVECTOR layout (include/psxsdk/libgte.h);
 * retyping it as the Sony type is Phase 2 work.  func_80022580 copies Unk80101EC8Record's +0xB8
 * and +0x104 as whole 16-byte VECTORs (pad included) and +0x1C8 as a whole 8-byte SVECTOR. */
"""


def h_game(s):
    s = rep(s, VEC_NOTE_OLD, VEC_NOTE_NEW)
    return s.replace("Vec4i32", "VECTOR")


def c31d3c(s):
    return s.replace("Vec4i32", "VECTOR")


def c9f9c(s):
    s = rep(s, "typedef Vec4i32 CamVec;", "typedef VECTOR CamVec;")
    return s.replace("Vec4i32", "VECTOR")


def c32d04(s):
    s = rep(s, "\ns32 D_800A3820;\n", "\nvoid **D_800A3820;\n")
    return fn(s, "func_80044504", lambda b: rep(b, *RESET))


def c3ab48(s):
    return fn(s, "func_80054F68", lambda b: rep(b, *RESET))


def c35000(s):
    return fn(s, "func_80044800", lambda b: push(b, "list"))


def c31548(s):
    return fn(s, "func_80040D48", lambda b: push(push(push(push(b, "list"), "list2"), "list3"), "list4"))


def c368e4(s):
    s = fn(s, "func_80046BF4", lambda b: rep(b, *RESET))
    s = fn(s, "func_800475A4", lambda b: push(b, "temp"))
    s = fn(s, "func_80047A90", lambda b: push(b, "temp"))
    s = fn(s, "func_80048BA4", lambda b: push(b, "list"))
    s = fn(s, "func_80049718", lambda b: push(b, "list"))
    s = fn(s, "func_80049A2C", lambda b: push(b, "list"))
    return s


def c2b344(s):
    s = rep(s, "extern s32 D_800A7EF0[];\nextern s32 *func_8003EB84(s32, s32, s32 *);\n",
            "extern Unk800A6690Rec *D_800A7EF0[]; /* func_8003E6D8's buffer of the queued Unk800A6690Rec */\n"
            "extern Unk800A6690Rec **func_8003EB84(s32, s32, Unk800A6690Rec **);\n")

    def e6d8(b):
        b = push(b, "list")
        b = rep(b, "    s32 *out;\n", "    Unk800A6690Rec **out;\n")
        b = rep(b, "                                *out++ = (s32)e2;\n", "                                *out++ = e2;\n")
        if "e6d8_reread" in OPT:
            b = rep(b, "        *(s32 *)D_800A3820 = *out;\n        (*(Unk800A6690Rec **)D_800A3820)->unk58 = 0;\n        D_800A3820 += 4;\n",
                    "        *D_800A3820 = *out;\n        (*out)->unk58 = 0;\n        D_800A3820++;\n")
        else:
            b = rep(b, "        *(s32 *)D_800A3820 = *out;\n        (*(Unk800A6690Rec **)D_800A3820)->unk58 = 0;\n        D_800A3820 += 4;\n",
                    "        *D_800A3820 = *out;\n        ((Unk800A6690Rec *)*D_800A3820)->unk58 = 0;\n        D_800A3820++;\n")
        return b

    def eb84(b):
        b = push(b, "list")
        b = rep(b, "s32 *func_8003EB84(s32 a0, s32 a1, s32 *out) {\n",
                "Unk800A6690Rec **func_8003EB84(s32 a0, s32 a1, Unk800A6690Rec **out) {\n")
        b = rep(b, "                                    *out = (s32)e2;\n", "                                    *out = e2;\n")
        return b
    s = fn(s, "func_8003E6D8", e6d8)
    s = fn(s, "func_8003EB84", eb84)
    return s


FILES = [("include/bb2.h", h_bb2), ("include/game.h", h_game), ("src/main/31D3C.c", c31d3c),
         ("src/main/9F9C.c", c9f9c), ("src/main/32D04.c", c32d04), ("src/main/3AB48.c", c3ab48),
         ("src/main/35000.c", c35000), ("src/main/31548.c", c31548), ("src/main/368E4.c", c368e4),
         ("src/main/2B344.c", c2b344)]


# B4 sweep of the moved bodies (abl_w2b7.py): drops at 0, labels for what scores, measured scores appended
# to the carried labels.
sys.path.insert(0, HERE)
import abl_w2b7 as ABL
DROP = ["1eb0_cam", "1eb0_cross", "4504_s0", "4504_v1", "4504_v0", "e6d8_cam", "3f08_first", "4f68_v3"]
LABELS = [
    ("src/main/31D3C.c", "    fp_ptr = D_800F62E0;\n",
     "fp_ptr holds D_800F62E0 for the loop and the first func_8004A1FC call; naming D_800F62E0 directly "
     "re-forms the address and the registers rotate (score 45)"),
    ("src/main/368E4.c", "    jb = Judge;\n",
     "jb holds the Judge base ahead of the loop; indexing Judge directly re-forms it in the loop (score 5)"),
    ("src/main/35000.c", "            cz = cos_val * sv.vz;\n",
     "cz / sz / cx / sx: the four products staged ahead of the sums; computed in the sums the multiplies "
     "reorder (score 61)"),
    ("src/main/35000.c", "            angle = rec->unk5C;\n",
     "angle read ahead of the sv.vz store; read at its uses the loads reorder (score 12)"),
    ("src/main/35000.c", "                last = D_800A9CF8.unk2 - 1;\n",
     "last staged ahead of the scan address; in the expression the subtraction moves (score 8)"),
    ("src/main/9F9C.c", "  s32 *scratch = (s32 *) 0x1F8001B0;\n",
     "the scratchpad addresses scratch_c (0x1F8001C0) / scratch_d (0x1F8001D0) and work (0x1F8002B8) are "
     "held in locals; as literals they are re-formed at each use (scratch_c: score 29; scratch_d: 17; "
     "work: 13)"),
    ("src/main/9F9C.c", "        perp = ang[0] + 0x400;\n",
     "perp staged ahead of the vc stores; in the Judge indices the addiu moves (score 3)"),
]
CARRIED = [
    ("src/main/2B344.c", "/* FAKE: the column shift is written in both arms", "score 9"),
    ("src/main/368E4.c", "/* FAKE: s16 temporary for the negated pitch", "score 3"),
    ("src/main/368E4.c", "/* FAKE: loop-note ref weighting lifts a3's", "score 10"),
    ("src/main/368E4.c", "/* FAKE: loop-note ref weighting lifts pa2", "score 13"),
    ("src/main/368E4.c", "/* FAKE: loop-note ref weighting keeps pt1", "score 8"),
    ("src/main/368E4.c", "/* FAKE: loop tail duplicated into both arms", "score 16"),
    ("src/main/9F9C.c", "/* FAKE: one local reused for four values", "one local per value: score 25"),
    ("src/main/9F9C.c", "/* FAKE (owner ruling Q90", "an s32 mask: score 3"),
    ("src/main/9F9C.c", "/* FAKE: named intermediate (no-new-park-categories.md entry 6): unk_6A", "score 3"),
    ("src/main/9F9C.c", "/* FAKE: named intermediate (no-new-park-categories.md entry 6): with", "score 11"),
    ("src/main/9F9C.c", "/* FAKE (gotos)", "the goto-free spelling: score 2; the range test spelled like the first: score 11"),
]


def add_score(s, anchor, txt):
    if s.count(anchor) != 1:
        raise SystemExit("carried anchor %r: %d" % (anchor, s.count(anchor)))
    i = s.index(anchor)
    j = s.index("*/", i)
    t = s[:j].rstrip()
    if not t.endswith("."):
        t += "."
    return t + " Ablated (2026-10-06): %s. " % txt + s[j:]


def sweep(p, s):
    byname = {a[0]: a for a in ABL.A}
    for d in DROP:
        name, fname, path, t = byname[d]
        if path == p:
            s = fn(s, fname, t)
    for f, anchor, text in LABELS:
        if f == p:
            ind = anchor[:len(anchor) - len(anchor.lstrip(" "))]
            s = rep(s, anchor, "%s/* FAKE: %s. */\n%s" % (ind, text, anchor))
    for f, anchor, txt in CARRIED:
        if f == p:
            s = add_score(s, anchor, txt)
    if p == "src/main/9F9C.c":
        s = rep(s, "/* func_8002304C (tanren_CameraControl) - pure C, no FAKE constructs.\n",
                "/* func_8002304C (tanren_CameraControl) - pure C; its scratchpad address holders are a labelled FAKE.\n")
    return s


def unbrace(b, first):
    """drop the bare `{ ... }` whose first statement starts with `first` and dedent its body by two"""
    i = b.index(first)
    o = b.rindex("\n", 0, i - 1) + 1          # the `{` line
    ind = b[o:b.index("{", o)]
    if b[o:i] != ind + "{\n":
        raise SystemExit("unbrace: no bare block before %r" % first)
    c = b.index("\n" + ind + "}\n", i) + 1    # the matching `}` line (same indent)
    body = b[i:c]
    body = "".join(l[2:] if l.startswith("  ") else l for l in body.splitlines(True))
    return b[:o] + body + b[c + len(ind) + 2:]


# rev-w2b7: B6 (D_800A3808 is u8 *, as g_gpu_ot_ptr) and the non-blocking fixes
def fix7(p, s):
    if p == "include/bb2.h":
        s = rep(s, "extern s32 D_800A3808;\n", "extern u8 *D_800A3808;\n")
    ot = ("D_800A3808 = (s32)g_gpu_ot_ptr;\n", "D_800A3808 = g_gpu_ot_ptr;\n")
    ot2 = ("D_800A378C = (u32 *)((s32)g_gpu_ot_ptr + 0x10);\n", "D_800A378C = (u32 *)(g_gpu_ot_ptr + 0x10);\n")
    if p in ("src/main/3AB48.c", "src/main/368E4.c"):
        f = "func_80054F68" if p == "src/main/3AB48.c" else "func_80046BF4"
        s = fn(s, f, lambda b: rep(rep(b, *ot), *ot2))
        if f == "func_80046BF4":   # count1 re-measured in place on this batch (fcam/abl_w2b8.py bf4_count1)
            s = rep(s, "the loads reorder (score 12). */", "the loads reorder (score 16). */")
    if p == "src/main/31D3C.c":
        s = fn(s, "func_80041EB0", lambda b: rep(rep(rep(b, "    s32 cos_val;\n", ""),
            "        cos_val = rcos(angle);\n", ""),
            "        {\n            tbl->light[1].pitch = (s16)-ratan2(dy, (cos_val * dz + rsin(angle) * dx) >> 12);\n        }\n",
            "        tbl->light[1].pitch = (s16)-ratan2(dy, (rcos(angle) * dz + rsin(angle) * dx) >> 12);\n"))
    if p == "src/main/2B344.c":
        s = fn(s, "func_8003EB84", lambda b: rep(rep(b, "    s16 temp_v0;\n", ""),
            "                    temp_v0 = D_800A7FE0[t4][t1];\n                    vidx = temp_v0;\n                    if (temp_v0 >= 0) {\n",
            "                    vidx = D_800A7FE0[t4][t1];\n                    if (vidx >= 0) {\n"))
    if p == "src/main/9F9C.c":
        def f304c(b):
            b = rep(b, "  s32 lim;\n", "  s32 work;\n")
            b = rep(b, "  lim = 0x1F8002B8;\n", "  work = 0x1F8002B8;\n")
            b = rep(b, "scratch, scratch_c, lim) == 0)", "scratch, scratch_c, work) == 0)")
            b = rep(b, "(s16 *)(scratch + 6), lim) == 0)", "(s16 *)(scratch + 6), work) == 0)")
            if "304c_vel" in OPT7:
                for off, c, i in (("0x10", "vx", 12), ("0x12", "vy", 13), ("0x14", "vz", 14)):
                    rd = "*((s16 *) (((u8 *) scratch) + %s))" % off
                    b = rep(b, "      vel = %s;\n      scratch[%d] = pos1->%s + (vel / 1024);\n" % (rd, i, c),
                            "      scratch[%d] = pos1->%s + (%s / 1024);\n" % (i, c, rd))
                    b = rep(b, "      vel = %s;\n      pos2->%s += vel / 1024;\n" % (rd, c),
                            "      pos2->%s += %s / 1024;\n" % (c, rd))
                b = rep(b, "      s16 vel;\n", "", 2)
            if "304c_vely" in OPT7:
                b = rep(b, "        s16 vel_y = *((s16 *) (((u8 *) scratch) + 0x12));\n        if (vel_y >= (-0x7FF))\n",
                        "        if (*((s16 *) (((u8 *) scratch) + 0x12)) >= (-0x7FF))\n")
            # the blocks that only scoped vel / vel_y
            for first in ("      scratch[12] = pos1->vx", "      pos2->vx += *((s16 *)", "        if (*((s16 *) (((u8 *) scratch) + 0x12))"):
                if first in b:
                    b = unbrace(b, first)
            return b
        s = fn(s, "func_8002304C", f304c)
    if p == "src/main/368E4.c" and "9a2c_p" in OPT7:
        s = fn(s, "func_80049A2C", lambda b: rep(rep(b, "        u8 *p = new_var6 + (arg0 * 2);\n", ""),
            "        temp_v1 = p[arg2];\n", "        temp_v1 = (new_var6 + (arg0 * 2))[arg2];\n"))
    return s


OPT7 = {"304c_vel", "304c_vely", "9a2c_p"}   # rev-w2b7's optional drops, each measured 0
for a in sys.argv[1:]:
    if a.startswith("opt7="):
        OPT7 = set(a[5:].split(",")) - {""}


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b7"])[0]
    for p, g in FILES:
        s = fix7(p, sweep(p, g(base(p))))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b7 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
