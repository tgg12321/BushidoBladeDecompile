#!/usr/bin/env python3
# Worker-2 batch 11: the asm-operand-free part of F02 (17AFC collision scratchpad).
# - func_8002FDB0: the character's scratchpad points SPAD->unkA8[index][1..3] (0x1F8000A8 + index * 0x108 + i * 0xC)
#   minus point 1 into the record's unkA8 / unkB8 (0x1F800360 / 0x1F800370), whose cross product the GTE stores
#   to unkC8 (0x1F800380); it returns unkC8.y > 0. The GTE islands keep their literal operands.
# - func_80029454: the halving loop walks ws (LeafPos *) two points at a time.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f02/w2b11.py [opt=...] [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "39c8ac8ed"   # batch 10 on p2/w2 (main 9462a0204)
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, fn = B2.rep, B2.fn

FDB0_OLD_HEAD = """    s32 stride;
    s32 v1, v2;
    s32 w1, w2;
    s32 ret;

    stride = arg0->index * 264;

    /* Compute (point_a - center) into scratchpad SCR[0x60..0x68] and
     * (point_b - center) into SCR[0x70..0x78].  Source vectors live at
     * stride-offset slots in scratchpad (0xB4/0xB8/0xBC = center xyz;
     * 0xC0/0xC4/0xC8 = a xyz; 0xCC/0xD0/0xD4 = b xyz). */
"""
FDB0_NEW_HEAD = """    LeafPos *pt;

    /* The character's points 2 and 3 relative to its point 1, into the collision record's unkA8 and
     * unkB8 (0x1F800360 / 0x1F800370, the GTE operands below); the GTE's product lands in unkC8. */
    pt = SPAD->unkA8[arg0->index];
"""


def fdb0(b):
    b = rep(b, FDB0_OLD_HEAD, FDB0_NEW_HEAD)
    i = b.index("    v1 = *(s32 *)((u8 *)0x1F8000C0 + stride);\n")
    j = b.index("    /* PsyQ libgte inline macro gte_SetRotMatrix")
    new = ("    SPAD->unk2B8.rec.unkA8.x = pt[2].x - pt[1].x;\n"
           "    SPAD->unk2B8.rec.unkA8.y = pt[2].y - pt[1].y;\n"
           "    SPAD->unk2B8.rec.unkA8.z = pt[2].z - pt[1].z;\n"
           "    SPAD->unk2B8.rec.unkB8.x = pt[3].x - pt[1].x;\n"
           "    SPAD->unk2B8.rec.unkB8.y = pt[3].y - pt[1].y;\n"
           "    SPAD->unk2B8.rec.unkB8.z = pt[3].z - pt[1].z;\n\n")
    old = b[i:j]
    if old.count("*(s32 *)0x1F8003") != 6:
        raise SystemExit("func_8002FDB0: unexpected head")
    b = b[:i] + new + b[j:]
    b = rep(b, "    /* Read MAC2 from scratchpad and return slt(0, MAC2) — i.e. MAC2 > 0. */\n    ret = *(s32 *)0x1F800384;\n",
            "    /* unkC8.y (MAC2) > 0 */\n    return 0 < SPAD->unk2B8.rec.unkC8.y;\n")
    b = rep(b, "    return 0 < ret;\n", "")
    # rev-w2b11 (N1): the ctc2 targets are $0 / $2 / $4
    b = rep(b, "     * rotation-matrix words at r into cop2 control regs R11R12/R13R21/R22R23.\n",
            "     * rotation-matrix words at r into cop2 control regs R11R12 / R22R23 / R33\n     * (ctc2 $0 / $2 / $4: the words holding the diagonal).\n")
    return b


def f9454(b):
    b = rep(b, "    s32 *p;\n", "    LeafPos *p;\n")
    b = rep(b, "            p = (s32 *)((u8 *)ws + (i * 0x60 + j * 0x18));\n", "            p = &ws[i * 8 + j * 2];\n")
    b = rep(b, "            p[0] >>= 1;\n            p[1] >>= 1;\n            p[2] >>= 1;\n"
               "            p[3] >>= 1;\n            p[4] >>= 1;\n            p[5] >>= 1;\n",
            "            p[0].x >>= 1;\n            p[0].y >>= 1;\n            p[0].z >>= 1;\n"
            "            p[1].x >>= 1;\n            p[1].y >>= 1;\n            p[1].z >>= 1;\n")
    return b


def c17afc(s):
    s = fn(s, "func_8002FDB0", fdb0)
    if "9454" in OPT:   # no typed form of the halving cursor matches (debt row)
        s = fn(s, "func_80029454", f9454)
    return s


FILES = [("src/main/17AFC.c", c17afc)]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b11"])[0]
    for p, g in FILES:
        B2.BASE = BASE
        s = g(B2.show(p))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b11 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
