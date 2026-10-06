#!/usr/bin/env python3
# Worker-2 batch 8: the two measured debt fixes from rev-w2b6.
# - func_80022408 takes the fighter's position as Vec3i32 * (it reads x and z) and its callers pass &unk_F4.
# - Rec44's +0x10 halfwords are an SVECTOR rotation (func_8001A538 builds the camera matrix from vx / vy / vz;
#   h16 had no reader or writer: the pad); func_80046BF4 takes (Vec3i32 *, SVECTOR *).
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/fcam/w2b8.py [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "eec262977"   # batch 7 on p2/w2 (main 84373e6f8)
rep, fn = B2.rep, B2.fn


def base(p):
    B2.BASE = BASE
    return B2.show(p)


ROT = (("h10", "unk_10.vx"), ("h12", "unk_10.vy"), ("h14", "unk_10.vz"))


def rot(s):
    """Rec44's h10 / h12 / h14 -> unk_10.vx / vy / vz (every h1x in the TUs passed here is a Rec44 field)."""
    for a, b in ROT:
        s = re.sub(r"\b%s\b" % a, b, s)
    return s


def h_game(s):
    s = rep(s, " * 12-byte vector: func_8001BC70 / func_8001BCF0 copy it as a struct (three lw, then three\n"
               " * sw through one base register); member-by-member copies compile differently.\n",
            " * 12-byte vector: func_8001BC70 / func_8001BCF0 copy it as a struct (three lw, then three\n"
            " * sw through one base register); member-by-member copies compile differently. +0x10 is the\n"
            " * camera rotation: func_8001A538 builds the matrix from vx / vy / vz and func_80046BF4 takes\n"
            " * it with the +0x00 vector; no field access reads or writes its pad (+0x16).\n")
    s = rep(s, "    s16 h10; s16 h12; s16 h14; s16 h16;\n", "    SVECTOR unk_10;\n")
    return s


def h_bb2(s):
    s = rep(s, "extern s32 func_80022408(s32 *);\n", "extern s32 func_80022408(Vec3i32 *);\n")
    s = rep(s, "extern void func_80046BF4(s32 *, s16 *, s32);\n", "extern void func_80046BF4(Vec3i32 *, SVECTOR *, s32);\n")
    return s


def c17afc(s):
    return fn(s, "func_800325E0", lambda b: rep(b, "D_800A36B4->h12;", "D_800A36B4->unk_10.vy;"))


def c368e4(s):
    def bf4(b):
        b = rep(b, "void func_80046BF4(s32 *a0, s16 *a1, s32 a2) {\n",
                "void func_80046BF4(Vec3i32 *a0, SVECTOR *a1, s32 a2) {\n")
        for i, f in enumerate("xyz"):
            b = b.replace("-a1[%d]" % i, "-a1->v%s" % f)
            b = rep(b, "result[%d] + a0[%d];" % (i, i), "result[%d] + a0->%s;" % (i, f))
        if "a1[" in b:
            raise SystemExit("func_80046BF4: a1[ left")
        return b
    return fn(s, "func_80046BF4", bf4)


BF4_CALL = ("func_80046BF4(&D_800F6608.unk_00.x, &D_800F6608.unk_10.vx, 0x2710);",
            "func_80046BF4(&D_800F6608.unk_00, &D_800F6608.unk_10, 0x2710);")


def c2b344(s):
    def c9a4(b):
        b = rep(rot(b), *BF4_CALL)
        return rep(b, "func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);", "func_80022408(&D_80101EC8[D_800A3748].unk_F4);")
    s = fn(s, "func_8003C9A4", c9a4)
    s = fn(s, "func_8003CD10", lambda b: rep(rot(b), *BF4_CALL))

    def ce18(b):
        b = rep(b, "        s32 *addr = &D_80101EC8[0].unk_F4.x;\n", "        Vec3i32 *addr = &D_80101EC8[0].unk_F4;\n")
        return rep(b, "            addr = &D_80101EC8[1].unk_F4.x;\n", "            addr = &D_80101EC8[1].unk_F4;\n")
    return fn(s, "func_8003CE18", ce18)


def unjoin(s, name):
    """a definition flattened onto one line (statements joined with their indentation kept) -> one statement per line"""
    i = s.index("\nvoid %s(" % name) + 1
    j = s.index("\n", i)
    parts = re.split(r"(?<=[;{}])(?= {4,}|}$)", s[i:j])
    return s[:i] + "\n".join(parts) + s[j:]


def c9f9c(s):
    s = rot(s)
    s = rep(s, "unk_10.vy/unk_10.vz", "unk_10.vy / vz", n=2)
    s = unjoin(s, "func_8001B294")
    s = rep(s, "func_80046BF4(&local.unk_00.x, &local.unk_10.vx, local.w18);",
            "func_80046BF4(&local.unk_00, &local.unk_10, local.w18);", n=2)
    s = rep(s, "func_80022408(&D_80101EC8[D_800A3748].unk_F4.x);", "func_80022408(&D_80101EC8[D_800A3748].unk_F4);", n=2)

    def f2408(b):
        b = rep(b, "s32 func_80022408(s32 *arg0) {\n", "s32 func_80022408(Vec3i32 *arg0) {\n")
        b = rep(b, "    t1 = arg0[0];\n    t2 = arg0[2];\n", "    t1 = arg0->x;\n    t2 = arg0->z;\n")
        return b
    return fn(s, "func_80022408", f2408)


FILES = [("include/game.h", h_game), ("include/bb2.h", h_bb2), ("src/main/17AFC.c", c17afc),
         ("src/main/368E4.c", c368e4), ("src/main/2B344.c", c2b344), ("src/main/9F9C.c", c9f9c)]


# B4 / B6 sweep of the moved bodies (abl_w2b8.py, scored on the batch in place): drops at 0, labels for
# what scores, measured scores appended to the carried labels.
sys.path.insert(0, HERE)
import abl_w2b8 as ABL
DROP = ["b294_dv", "b478_z", "b478_res", "b748_cast", "b748_avg", "b748_sum", "e404_cast", "e6e4_cast", "c9a4_cast",
        "cd10_cast", "ce18_v0", "ce18_val", "ce18_val2", "a820_tbl"]
LABELS = [
    ("src/main/9F9C.c", "    s32 cur;\n    s32 use_high;\n",
     "cur / t are reused for the x, y and z steps, the pitch target and the w18 step; one local per role "
     "re-seats the products and moves the loads (score 23)"),
    ("src/main/9F9C.c", "        s32 t1 = a0->unk_F4.z;\n",
     "t1 / t2 read the two z's ahead of the unk_10.vx store; read in the sum, the store is scheduled "
     "ahead of the loads (score 4)"),
    ("src/main/9F9C.c", "    Rec44 *s2 = &D_800F5328;\n",
     "s2 holds &D_800F5328 in a saved register; through the symbol each access re-forms the address "
     "(score 34)"),
    ("src/main/9F9C.c", "                s16 cnt = counter - 1;\n",
     "cnt computed ahead of the unk_10.vy store; computed at the D_800A36FC store the addiu moves below "
     "it (score 6)"),
    ("src/main/9F9C.c", "        zval = (frac * (a->h8)) + (inv_frac * (b->h8));\n",
     "zval's products formed ahead of the unk_10 stores; formed at the unk_00.z store the multiply "
     "chains reorder (score 35)"),
    ("src/main/9F9C.c", "    v = math_SignExt12Div(val - dst->unk_10.vy, 0x10);\n",
     "v computed ahead of the unk_10.vz store; computed in the unk_10.vy store the vz store rises "
     "above the call (score 4)"),
]
CARRIED = [
    ("src/main/9F9C.c", "/* FAKE: cancellation pair", "score 29"),
    ("src/main/9F9C.c", "/* FAKE: second handle to D_800A37D2",
     "both sites without p (indexing &D_800A37D2): score 17; each byte by its own symbol: score 27"),
]


def add_score(s, anchor, txt):
    if s.count(anchor) != 1:
        raise SystemExit("carried anchor %r: %d" % (anchor, s.count(anchor)))
    i = s.index(anchor)
    j = s.index("*/", i)
    return s[:j].rstrip() + " Ablated (2026-10-06): %s. " % txt + s[j:]


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
    if p == "src/main/9F9C.c":   # the block that held sum / avg
        s = rep(s, "    {\n        if ((base->unk_198[0].y + base->unk_198[1].y) / 2 - base->unk_180.y < 0xC8) {\n"
                   "            D_800A3310 += 1;\n        }\n    }\n",
                "    if ((base->unk_198[0].y + base->unk_198[1].y) / 2 - base->unk_180.y < 0xC8) {\n"
                "        D_800A3310 += 1;\n    }\n")
    return s


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b8"])[0]
    for p, g in FILES:
        s = g(base(p))
        if "nosweep" not in sys.argv[1:]:
            s = sweep(p, s)
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b8 wrote %d files to %s" % (len(FILES), "the tree" if apply else out))


if __name__ == "__main__":
    main()
