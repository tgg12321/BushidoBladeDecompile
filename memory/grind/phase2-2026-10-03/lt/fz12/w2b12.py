#!/usr/bin/env python3
# Worker-2 batch 12 (closing-census items 1-5):
# - libgpu.h gains PsyQ LIBGPU.H's getaddr / addPrim macros; 2B344 func_8003DBE4 and 368E4 func_80048FFC link their
#   DR_MOVEs into the D_800A378C ordering table with addPrim instead of the expanded P_TAG views.
# - 31D3C StageLight is a table of six-word rows (0x18 bytes: four colour words, the angle word, a parameter word).
# - 368E4 g_cam_bone_data is a MATRIX: camera_InitBoneData copies D_80101DF0.xf.mat into it and halves its second
#   row; D_800EEDD6 / D_800EEDD8 (m[1][0] / m[1][1]) fold into it.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/fz12/w2b12.py [opt=...] [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "ebacf0123"   # main (batches 9-11 and fot1 landed)
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, fn = B2.rep, B2.fn


def h_libgpu(s):
    return rep(s, "#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))\n",
               "#define setaddr(p, _addr) (((P_TAG *)(p))->addr = (u32)(_addr))\n"
               "#define getaddr(p) (u32)(((P_TAG *)(p))->addr)\n"
               "#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)\n")


SOTN = "/* SOTN: include/psxsdk/libgpu.h:88 @db41b28eee52969244a52cc269c8163d1ed8826a (PS1 use: src/main/psxsdk/libgpu/sys.c:288) */\n"


def link(b, ind, ot, p, n=1):
    return rep(b, "%s((OTag *)%s)->addr = ((OTag *)&D_800A378C[%s])->addr;\n%s((OTag *)&D_800A378C[%s])->addr = (u32)%s;\n"
               % (ind, p, ot, ind, ot, p), "%saddPrim(&D_800A378C[%s], %s);\n" % (ind, ot, p), n)


def c2b344(s):
    def dbe4(b):
        b = rep(b, "                /* FAKE: SDK addPrim (setaddr/getaddr P_TAG views) on OT entry idx; the explicit\n"
                   "                 * mask-and-or spelling of the two stores scores 14. */\n"
                   "                " + SOTN, "")
        b = link(b, "                ", "idx", "pkt")
        b = link(b, "        ", "0xFFB", "pkt")
        # rev-w2b12 (N1): the guarded do-while is a plain for loop (sandbox 0)
        b = rep(b, "    i = 0;\n    pkt += D_800A36AC & 1;\n\n    if (i < limit) {\n        do {\n",
                "    pkt += D_800A36AC & 1;\n\n    for (i = 0; i < limit; i++) {\n")
        i = b.index("    for (i = 0; i < limit; i++) {\n")
        j = b.index("            arg0 += step;\n            i++;\n        } while (i < limit);\n    }\n", i)
        body = b[i + len("    for (i = 0; i < limit; i++) {\n"):j]
        body = "".join(l[4:] if l.startswith("    ") else l for l in body.splitlines(True))
        return (b[:i] + "    for (i = 0; i < limit; i++) {\n" + body + "        arg0 += step;\n    }\n"
                + b[j + len("            arg0 += step;\n            i++;\n        } while (i < limit);\n    }\n"):])
    return fn(s, "func_8003DBE4", dbe4)


def c368e4(s):
    def ffc(b):
        b = rep(b, "        /* FAKE: SDK addPrim (setaddr/getaddr P_TAG views) on OT entry 0xFFF. */\n        " + SOTN, "")
        b = rep(b, "         * directly, an s32 copy or a (s16) cast drops the t1 copy. */\n",
                "         * directly, an s32 copy or a (s16) cast drops the t1 copy. Ablated (2026-10-06): score 57. */\n")
        return link(b, "        ", "0xFFF", "p", 2)
    s = fn(s, "func_80048FFC", ffc)
    s = rep(s, "extern u8 g_cam_bone_data;\n", "extern MATRIX g_cam_bone_data;\n")
    old_i = s.index("typedef struct { s16 lo; s16 hi; } CamHalves;\n")
    old_j = s.index("void camera_InitBoneData(void) {")
    s = s[:old_i] + s[old_j:]
    new = ("void camera_InitBoneData(void) {\n"
           "    g_cam_bone_data = D_80101DF0.xf.mat;\n"
           "    g_cam_bone_data.m[1][0] >>= 1;\n"
           "    g_cam_bone_data.m[1][1] >>= 1;\n"
           "    g_cam_bone_data.m[1][2] >>= 1;\n"
           "}\n")
    if "ibd_fence" in OPT:
        new = new.replace("    g_cam_bone_data = D_80101DF0.xf.mat;\n", "    do { g_cam_bone_data = D_80101DF0.xf.mat; } while (0);\n")
    if "ibd_hold" in OPT:
        new = new.replace("    g_cam_bone_data.m[1][0] >>= 1;\n    g_cam_bone_data.m[1][1] >>= 1;\n",
                          "    {\n        s16 h0 = g_cam_bone_data.m[1][0];\n        s16 h1 = g_cam_bone_data.m[1][1];\n"
                          "        g_cam_bone_data.m[1][0] = h0 >> 1;\n        g_cam_bone_data.m[1][1] = h1 >> 1;\n    }\n")
    i = s.index("void camera_InitBoneData(void) {")
    j = s.index("\n}\n", i) + 3
    return s[:i] + new + s[j:]


def c31d3c(s):
    s = rep(s, "extern s32 StageLight[];\n", "extern s32 StageLight[][6]; /* 0x18-byte rows: four colour words, the angle word, a parameter */\n")

    def f1c8(b):
        b = rep(b, "    s32 *p = (s32 *)((u8 *)StageLight + a0 * 24);\n", "    s32 *p = StageLight[a0];\n")
        b = rep(b, "    func_80042478(*(s32 *)((u8 *)p + 4));\n", "    func_80042478(p[1]);\n")
        # B4: val held the yaw word and then the pitch half; one local per value scores 0
        b = rep(b, "    s32 val;\n", "    s32 yaw;\n    s32 pitch;\n")
        b = rep(b, "    val = *p;\n", "    yaw = *p;\n")
        # rev-w2b12 (B5): the pitch half by shift (combine narrows it to lh 2)
        b = rep(b, "    val = *(s16 *)((u8 *)p + 2);\n", "    pitch = *p >> 16;\n")
        b = rep(b, "light[0].yaw = val & 0xFFF;", "light[0].yaw = yaw & 0xFFF;", 3)
        return rep(b, "light[0].pitch = val & 0xFFF;", "light[0].pitch = pitch & 0xFFF;", 3)
    return fn(s, "func_800421C8", f1c8)


FILES = [("include/psxsdk/libgpu.h", h_libgpu), ("src/main/2B344.c", c2b344), ("src/main/368E4.c", c368e4),
         ("src/main/31D3C.c", c31d3c)]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b12"])[0]
    for p, g in FILES:
        B2.BASE = BASE
        s = g(B2.show(p))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b12 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
