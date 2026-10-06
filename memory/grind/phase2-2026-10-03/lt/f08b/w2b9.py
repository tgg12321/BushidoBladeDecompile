#!/usr/bin/env python3
# Worker-2 batch 9: the F08 remainder in 309CC.
# - D_80094B96: the 21 ten-byte node-template records func_800408F8 / func_80040CB8 walk (stride 0xA up to
#   D_80094C68): +0 the parent node index (-1: none), +2..+6 the three Q0 position components func_800408F8
#   scales by unk_12, +8 the id func_80040CB8 copies into unk_8B4[] (-1: skipped). One extern replaces the
#   five per-field symbols D_80094B96 / B98 / B9A / B9C / B9E.
# - func_80040400's a1 walks Unk80045878Node records (0x68; fields +0 / +1 / +2..+A / +0xC / +0x58).
# - func_80040B44's t3 is a u16 cursor (the id / index stream at the resource's +8 offset).
# - func_80040CB8's slot is the Unk80045878Node it fills.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f08b/w2b9.py [opt=...] [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "9462a0204"   # main (batch 8 landed)
OPT = {"rec_idx", "b44_seen"}   # the measured forms; opt= replaces the set
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT = set(a[4:].split(",")) - {""}
rep, fn = B2.rep, B2.fn


def base(p):
    B2.BASE = BASE
    return B2.show(p)


REC = """/* The 21 ten-byte node-template records at 0x80094B96 (up to D_80094C68): func_800408F8 builds
 * Unk80045878Obj.unk_2C[i] from record i (unk_00: the parent node index, -1 for none; unk_02: the
 * position, scaled by unk_12), func_80040CB8 copies unk_08 (-1: no node) into unk_8B4[]. */
typedef struct {
    s16 unk_00;
    s16 unk_02[3];
    s16 unk_08;
} Unk80094B96Rec;

"""


def h_game(s):
    a = "/* The per-player model object func_80045878 builds"
    return rep(s, a, REC + a)


F400 = """void func_80040400(Unk80045878Node *a0, Unk80045878Node *a1, s16 a2) {
    while (a1->node.unk2 != -1) {
        a1++;
    }
    a1->node.unk2 = 2;
    a1->node.unk0 = 3;
    a1->node.unk1 = 0;
    a1->node.unkC = &a0[6].node;
    a1->node.unk6 = 1;
    a1->node.unk8 = 0;
    a1->node.unkA = 0;
    a1->node.unk4 = a2;
    a1->unk58 = 0;
    a1[1].node.unk2 = -1;
}
"""


def c309cc(s):
    def f400(b):
        # rev-w2b9: the v1 / v0 / goto / a1++ / a1-- cluster ablates to 0 as a plain while loop
        i = b.index("void func_80040400(")
        return F400 + b[b.index("\n}\n", i) + 3:]
    s = fn(s, "func_80040400", f400)

    s = rep(s, "extern s16 D_80094B96[];\nextern s16 D_80094B98[];\nextern s16 D_80094B9A[];\nextern s16 D_80094B9C[];\n",
            "extern Unk80094B96Rec D_80094B96[21];\n")

    def f8f8(b):
        if "rec_idx" in OPT:
            b = rep(b, "        s32 off = 0;\n", "        s32 i = 0;\n")
            b = rep(b, "            v1 = *(s16 *)((u8 *)D_80094B96 + off);\n", "            v1 = D_80094B96[i].unk_00;\n")
            for k, sym in enumerate(("D_80094B98", "D_80094B9A", "D_80094B9C")):
                b = rep(b, "((s32)*(s16 *)((u8 *)%s + off) * a0->unk_12)" % sym, "(D_80094B96[i].unk_02[%d] * a0->unk_12)" % k)
            b = rep(b, "            off += 0xA;\n", "")
            b = rep(b, "        } while (off < 0xD2);\n", "            i++;\n        } while (i < 21);\n")
            return b
        if "rec_ptr" in OPT:
            b = rep(b, "        s32 off = 0;\n", "        Unk80094B96Rec *rec = D_80094B96;\n")
            b = rep(b, "            v1 = *(s16 *)((u8 *)D_80094B96 + off);\n", "            v1 = rec->unk_00;\n")
            for i, sym in enumerate(("D_80094B98", "D_80094B9A", "D_80094B9C")):
                b = rep(b, "((s32)*(s16 *)((u8 *)%s + off) * a0->unk_12)" % sym, "((s32)rec->unk_02[%d] * a0->unk_12)" % i)
            b = rep(b, "            off += 0xA;\n", "            rec++;\n")
            b = rep(b, "        } while (off < 0xD2);\n", "        } while (rec < &D_80094B96[21]);\n")
        else:
            b = rep(b, "            v1 = *(s16 *)((u8 *)D_80094B96 + off);\n",
                    "            v1 = ((Unk80094B96Rec *)((u8 *)D_80094B96 + off))->unk_00;\n")
            for i, sym in enumerate(("D_80094B98", "D_80094B9A", "D_80094B9C")):
                b = rep(b, "((s32)*(s16 *)((u8 *)%s + off) * a0->unk_12)" % sym,
                        "((s32)((Unk80094B96Rec *)((u8 *)D_80094B96 + off))->unk_02[%d] * a0->unk_12)" % i)
        return b
    s = fn(s, "func_800408F8", f8f8)

    def fb44(b):
        if "b44_seen" in OPT:
            b = rep(b, "    {\n        s32 *p1;\n        i = 0x11;\n        p1 = &seen[17];\n        do {\n            *p1 = 0;\n"
                       "            i--;\n            p1--;\n        } while (i >= 0);\n    }\n",
                    "    i = 0x11;\n    do {\n        seen[i] = 0;\n        i--;\n    } while (i >= 0);\n")
        b = rep(b, "    s32 *t3;\n", "    u16 *t3;\n")
        b = rep(b, "    t3 = (s32 *)((u8 *)v1 + *(s32 *)((u8 *)v1 + 8));\n", "    t3 = (u16 *)((u8 *)v1 + v1[2]);\n")
        b = rep(b, "    a0_val = *(u16 *)t3;\n", "    a0_val = *t3;\n", 2)
        b = rep(b, "            t3 = (s32 *)((u8 *)t3 + 2);\n            a3 = *(u16 *)t3;\n            t3 = (s32 *)((u8 *)t3 + 2);\n",
                "            t3++;\n            a3 = *t3;\n            t3++;\n")
        return b
    s = fn(s, "func_80040B44", fb44)

    if "cb8_notbl" not in OPT:
        s = rep(s, "extern s16 D_80094B9E[];\n", "")

    def fcb8(b):
        if "cb8_noslot" not in OPT:
            b = fcb8_slot(b)
        if "cb8_notbl" not in OPT:
            b = fcb8_tbl(b)
        return b

    def fcb8_tbl(b):
        if "cb8_idx" in OPT:
            b = rep(b, "    s16 *tbl;\n", "")
            b = rep(b, "    tbl = D_80094B9E;\n", "")
            b = rep(b, "        id = *tbl;\n", "        id = D_80094B96[i].unk_08;\n")
            b = rep(b, "        tbl = (s16 *)((s32)tbl + 0xA);\n", "")
            return b
        if "cb8_recptr" in OPT:
            b = rep(b, "    s16 *tbl;\n", "    Unk80094B96Rec *tbl;\n")
            b = rep(b, "    tbl = D_80094B9E;\n", "    tbl = D_80094B96;\n")
            b = rep(b, "        id = *tbl;\n", "        id = tbl->unk_08;\n")
            b = rep(b, "        tbl = (s16 *)((s32)tbl + 0xA);\n", "        tbl++;\n")
            return b
        b = rep(b, "    tbl = D_80094B9E;\n", "    tbl = &D_80094B96[0].unk_08;\n")
        b = rep(b, "        tbl = (s16 *)((s32)tbl + 0xA);\n", "        tbl = (s16 *)((u8 *)tbl + sizeof(Unk80094B96Rec));\n")
        return b

    def fcb8_slot(b):
        b = rep(b, "    s8 *slot = (s8 *)arg0->unk_8B4;\n", "    Unk80045878Node *slot = arg0->unk_8B4;\n")
        b = rep(b, "            *slot = kind;\n", "            slot->node.unk0 = kind;\n")
        b = rep(b, "                slot += 0x68;\n", "                slot++;\n")
        b = rep(b, "    *(s16 *)((s32)slot + 2) = -1;\n", "    slot->node.unk2 = -1;\n")
        return b
    s = fn(s, "func_80040CB8", fcb8)
    return sweep(s)


# B4 sweep (abl_w2b9.py, in place): func_800408F8's neg1 holder now ablates to 0 (dropped with its label);
# func_80040CB8's unk_08 cursor scores and is labelled.
def sweep(s):
    s = fn(s, "func_800408F8", lambda b: rep(rep(rep(rep(b,
        "        /* FAKE: constant holder for -1 (the parent-less marker); the literal scores 2 (li t0,-1 moves one slot) */\n", ""),
        "        s32 neg1 = -1;\n", ""), "p->node.unk2 = neg1;", "p->node.unk2 = -1;"), "if (v1 != neg1) {", "if (v1 != -1) {"))
    s = fn(s, "func_80040CB8", lambda b: rep(b, "    tbl = &D_80094B96[0].unk_08;\n",
        "    /* FAKE: tbl walks the records' unk_08 members (a cursor at +8, stepped by a record): a record\n"
        "       pointer reads unk_08 at 8(a3) off D_80094B96 (score 2), D_80094B96[i].unk_08 scores 21. */\n"
        "    tbl = &D_80094B96[0].unk_08;\n"))
    return s

FILES = [("include/game.h", h_game), ("src/main/309CC.c", c309cc)]


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b9"])[0]
    for p, g in FILES:
        s = g(base(p))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b9 wrote %d files to %s %s" % (len(FILES), "the tree" if apply else out, sorted(OPT)))


if __name__ == "__main__":
    main()
