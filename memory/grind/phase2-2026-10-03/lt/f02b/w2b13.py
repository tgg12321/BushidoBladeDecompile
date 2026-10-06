#!/usr/bin/env python3
# Worker-2 batch 13 (owner rulings Q115 / Q117):
# - Q115: 17AFC func_8002CD58 / func_8002DAD0 / func_8002D780 take the collision record as Unk1F8002B8Rec *, read
#   and write it by member, and their GTE island operand EXPRESSIONS name the same addresses as member addresses
#   (`obj + 0xA8` -> `&obj->unkA8`). Island template text, constraints, clobbers and operand order are untouched.
# - Q117: func_80031B24's two `&D_800A37E8` passes carry Q96's FAKE label.
# usage (repo root): python3 memory/grind/phase2-2026-10-03/lt/f02b/w2b13.py [out=DIR | apply]
import os, re, sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f09"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import w2b2 as B2
sys.argv = _argv
NL = chr(10)
BASE = "643796e7f"   # main (rules Q115-Q117; Q116 landed)
rep, fn = B2.rep, B2.fn
XYZ = "xyz"

# Unk1F8002B8Rec (game.h): +0x60 unk60[3] (LeafPos *), +0xA8 unkA8, +0xB8 unkB8, +0xC8 unkC8 (Vec3i32),
# +0xD8 unkD8 (MATRIX), +0xF8 unkF8 (SVECTOR), +0x100 unk100[2], +0x118 unk118[3] (Vec3i32)
VEC = [(0xA8, "obj->unkA8"), (0xB8, "obj->unkB8"), (0xC8, "obj->unkC8"),
       (0x100, "obj->unk100[0]"), (0x10C, "obj->unk100[1]"),
       (0x118, "obj->unk118[0]"), (0x124, "obj->unk118[1]"), (0x130, "obj->unk118[2]")]


def vec_at(off):
    for base, name in VEC:
        if off == base:
            return name
    raise SystemExit("no vector at 0x%X" % off)


def s32_at(off):
    for base, name in VEC:
        if base <= off < base + 12 and (off - base) % 4 == 0:
            return "%s.%s" % (name, XYZ[(off - base) // 4])
    raise SystemExit("no s32 member at 0x%X" % off)


def s16_at(off):
    if 0xD8 <= off < 0xEA:
        k = (off - 0xD8) // 2
        return "obj->unkD8.m[%d][%d]" % (k // 3, k % 3)
    if 0xF8 <= off < 0xFE:
        return "obj->unkF8.%s" % ("vx", "vy", "vz")[(off - 0xF8) // 2]
    raise SystemExit("no s16 member at 0x%X" % off)


def retype(b):
    b = re.sub(r"\(\*\(s32 \*\*\)\(obj \+ 0x6([048])\)\)\[([012])\]",
               lambda m: "obj->unk60[%d]->%s" % ("048".index(m.group(1)), XYZ[int(m.group(2))]), b)
    # GTE island operands: the same addresses as member addresses
    b = re.sub(r'"r"\(\(s32 \*\)\(obj \+ (0x[0-9A-F]+)\)\)',
               lambda m: '"r"((s32 *)&%s)' % (vec_at(int(m.group(1), 16)) if int(m.group(1), 16) != 0xF8 else "obj->unkF8"), b)
    b = re.sub(r'"r"\(obj \+ (0x[0-9A-F]+)\)',
               lambda m: '"r"(&%s)' % ("obj->unkD8" if int(m.group(1), 16) == 0xD8 else vec_at(int(m.group(1), 16))), b)
    b = re.sub(r"\(MATRIX \*\)\(obj \+ 0xD8\)", "&obj->unkD8", b)
    b = re.sub(r"\*\(s32 \*\)\(obj \+ (0x[0-9A-F]+)\)", lambda m: s32_at(int(m.group(1), 16)), b)
    b = re.sub(r"\(s32 \*\)\(obj \+ (0x[0-9A-F]+)\)", lambda m: "&%s" % s32_at(int(m.group(1), 16)), b)
    b = re.sub(r"\*\(s16 \*\)\(obj \+ (0x[0-9A-F]+)\)", lambda m: s16_at(int(m.group(1), 16)), b)
    if "(obj +" in b or "obj + 0x" in b.replace("/*", "\0").split("\0")[0]:
        left = [l for l in b.splitlines() if "(obj + 0x" in l and "/*" not in l and "*" != l.strip()[:1]]
        if left:
            raise SystemExit("raw obj views left: %s" % left[:3])
    return b


COMMENTS = [("gte_ApplyRotMatrix(obj + 0xF8, obj + 0x100)", "gte_ApplyRotMatrix(&obj->unkF8, &obj->unk100[0])"),
            ("MAC1-MAC3 to obj+0xC8.", "MAC1-MAC3 to obj->unkC8."),
            ("the squares to obj+0x100.", "the squares to obj->unk100[0]."),
            ("gte_SetRotMatrix(obj+0xD8)", "gte_SetRotMatrix(&obj->unkD8)"),
            ("product) to obj+0xC8/CC/D0.", "product) to obj->unkC8."),
            ("identity 3x3 rotation matrix at obj+0xD8", "identity 3x3 rotation matrix obj->unkD8")]


def comments(b):
    for a, c in COMMENTS:
        b = b.replace(a, c)
    return b


Q117 = ("%s/* FAKE: D_800A37E8 / EA / EC are one s16 x,y,z vector, reached by the first's address and\n"
        "%s * handed to %s as its s16 vector (owner rulings Q96 / Q117; the s16[3] and {x,y,z}\n"
        "%s * forms score 2 in func_80027AD8). */\n")


def label(ind, callee):
    return Q117 % (ind, ind, callee, ind)


def b24(b):
    # Q117: the label sits at each argument
    b = rep(b, "        func_800274BC((s32 *)&obj->vel, &D_800A37E8);\n",
            "        func_800274BC((s32 *)&obj->vel,\n" + label(" " * 22, "func_800274BC")
            + " " * 22 + "&D_800A37E8);\n")
    return rep(b, "            func_80032854(other ^ 1, 0xE, &SPAD->unkA8[other][j].x, &D_800A37E8);\n",
               "            func_80032854(other ^ 1, 0xE, &SPAD->unkA8[other][j].x,\n" + label(" " * 26, "func_80032854")
               + " " * 26 + "&D_800A37E8);\n")


HEAD_OLD = """/* Orients a triangle's local frame. The three vertex pointers at obj+0x60/
 * 0x64/0x68 give edge vectors a = v1 - v0 (obj+0xA8) and b = v2 - v0
 * (obj+0xB8); the GTE outer product n = a x b lands in obj+0xC8. If every
 * component of n is within +-0x3FFF and |n| (GTE SQR, then the g_sqrt_table_u8
 * byte-LUT integer sqrt with the GTE leading-zero count for large inputs) is
 * below 0x4000, the yaw (obj+0xFA) and pitch (obj+0xF8) are taken from a;
 * otherwise n is scaled down by 64 in place and the angles are taken from n.
 * Either way a rotation matrix is built at obj+0xD8 (identity, RotMatrixY by
"""
HEAD_NEW = """/* Orients a triangle's local frame. The three vertex pointers obj->unk60[0..2]
 * give edge vectors a = v1 - v0 (obj->unkA8) and b = v2 - v0 (obj->unkB8);
 * the GTE outer product n = a x b lands in obj->unkC8. If every
 * component of n is within +-0x3FFF and |n| (GTE SQR, then the g_sqrt_table_u8
 * byte-LUT integer sqrt with the GTE leading-zero count for large inputs) is
 * below 0x4000, the yaw (obj->unkF8.vy) and pitch (.vx) are taken from a;
 * otherwise n is scaled down by 64 in place and the angles are taken from n.
 * Either way a rotation matrix is built in obj->unkD8 (identity, RotMatrixY by
"""


def c17afc(s):
    s = rep(s, HEAD_OLD, HEAD_NEW)
    s = rep(s, "extern s32 func_8002DAD0(u8 *obj);\n", "extern s32 func_8002DAD0(Unk1F8002B8Rec *obj);\n")
    s = rep(s, "extern s32 func_8002CD58(u8 *obj);\n", "extern s32 func_8002CD58(Unk1F8002B8Rec *obj);\n")
    s = rep(s, "extern s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);\n",
            "extern s32 func_8002D780(s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq);\n")
    s = rep(s, "func_8002DAD0((u8 *)scr)", "func_8002DAD0(scr)", 2)
    s = rep(s, "func_8002CD58((u8 *)scr)", "func_8002CD58(scr)")
    s = rep(s, "func_8002D780(0, (u8 *)scr,", "func_8002D780(0, scr,")
    s = rep(s, "func_8002D780(1, (u8 *)scr,", "func_8002D780(1, scr,")
    s = fn(s, "func_80031B24", b24)
    s = fn(s, "func_8002CD58", lambda b: comments(retype(rep(b, "s32 func_8002CD58(u8 *obj) {", "s32 func_8002CD58(Unk1F8002B8Rec *obj) {"))))
    s = fn(s, "func_8002DAD0", lambda b: comments(retype(rep(b, "s32 func_8002DAD0(u8 *obj) {", "s32 func_8002DAD0(Unk1F8002B8Rec *obj) {"))))
    s = fn(s, "func_8002D780", lambda b: comments(retype(rep(b, "s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {",
                                                    "s32 func_8002D780(s32 flag, Unk1F8002B8Rec *obj, s32 *pos, s32 threshold, s32 r_sq) {"))))
    return s


FILES = [("src/main/17AFC.c", c17afc)]

# B4 sweep (abl_w2b13.py, scored in place on the retyped bodies): drops at 0; the Ruling-11 reused locals and
# the ax holder score and are labelled; measured scores appended to the carried FAKEs.
sys.path.insert(0, HERE)
import abl_w2b13 as ABL
DROP = ["cd58_tbl1", "cd58_tbl2", "cd58_yaw_up", "cd58_pitch", "cd58_npitch", "dad0_tbl", "dad0_angle2_up",
        "d780_az", "d780_bz"]
RELABEL = [  # (function, comment start, FAKE text inserted after "/* ", score text appended)
    ("func_8002CD58", "/* temp holds two values (Ruling 11):", "FAKE: ", "one local per value: score 6"),
    ("func_8002CD58", "/* len holds two values (Ruling 11):", "FAKE: ", "one local per value: score 3"),
    ("func_8002D780", "/* cross_center and cross_point each hold three values", "FAKE: ", "a pair per side test: score 36"),
    ("func_8002DAD0", "/* FAKE: the scaled Z delta", "", "a block-local dz: score 6"),
    ("func_8002D780", "/* FAKE: the third edge test's", "", "a block-local dz: score 9"),
    ("func_8002D780", "/* FAKE: same-value re-store", "", "without the re-store: score 4"),
]
AX = ("                s32 ax = cx - x0;\n",
      "                /* FAKE: ax holds cx - x0 ahead of the dx / az set-up; in the product the subtraction\n"
      "                 * moves (score 2). */\n                s32 ax = cx - x0;\n")
AX = (AX[0], AX[1].replace("ahead of the dx / az set-up", "ahead of the dx set-up"))


def sweep(s):
    byname = {a[0]: a for a in ABL.A}
    for d in DROP:
        _n, f, t = byname[d]
        s = fn(s, f, t)
    for f, start, pre, score in RELABEL:
        def lab(b, start=start, pre=pre, score=score):
            i = b.index(start)
            j = b.index("*/", i)
            body = b[i:j].rstrip()
            if not body.endswith("."):
                body += "."
            return b[:i] + "/* " + pre + body[3:] + " Ablated (2026-10-06): %s. " % score + b[j:]
        s = fn(s, f, lab)
    s = fn(s, "func_8002D780", lambda b: rep(b, *AX))
    return fix13(s)


# rev-w2b13: B4 (nyaw / angle1 single-use holders: 0), B5 / B7 (p118 / p124 / p10C are Vec3i32 * holders written
# by member, passed to func_8002D518 as its s32 *: 0), B6 (no (s32 *) on the D780 island operands: the "r"
# operand takes the address as is).
def fix13(s):
    def cd58(b):
        b = rep(rep(b, "    s32 nyaw;\n", ""), "    nyaw = ratan2(obj->unkC8.x, obj->unkC8.z);\n",
                "    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);\n")
        return rep(b, "    obj->unkF8.vy = 0x800 - nyaw;\n", "")

    def dad0(b):
        b = rep(rep(b, "    s32 angle1;\n", ""), "    angle1 = ratan2(obj->unkC8.x, obj->unkC8.z);\n",
                "    obj->unkF8.vy = 0x800 - ratan2(obj->unkC8.x, obj->unkC8.z);\n")
        return rep(b, "    obj->unkF8.vy = 0x800 - angle1;\n", "")

    def d780(b):
        b = rep(b, '"r"((s32 *)&obj->unkF8)', '"r"(&obj->unkF8)')
        b = rep(b, '"r"((s32 *)&obj->unk100[0])', '"r"(&obj->unk100[0])')
        for p in ("p118", "p124", "p10C"):
            b = rep(b, "        s32 *%s;\n" % p, "        Vec3i32 *%s;\n" % p)
        b = rep(b, "        p118 = &obj->unk118[0].x;\n        p124 = &obj->unk118[1].x;\n",
                "        p118 = &obj->unk118[0];\n        p124 = &obj->unk118[1];\n")
        b = rep(b, "        p10C = &obj->unk100[1].x;\n", "        p10C = &obj->unk100[1];\n")
        for p in ("p118", "p124", "p10C"):
            b = b.replace("%s[0] = " % p, "%s->x = " % p).replace("%s[1] = " % p, "%s->y = " % p)
        b = rep(b, "func_8002D518(sqrt_val, dist, p118, p124)", "func_8002D518(sqrt_val, dist, (s32 *)p118, (s32 *)p124)")
        b = rep(b, "func_8002D518(sqrt_val, dist, p10C, p118)", "func_8002D518(sqrt_val, dist, (s32 *)p10C, (s32 *)p118)")
        return rep(b, "func_8002D518(sqrt_val, dist, p10C, p124)", "func_8002D518(sqrt_val, dist, (s32 *)p10C, (s32 *)p124)")
    s = fn(s, "func_8002CD58", cd58)
    s = fn(s, "func_8002DAD0", dad0)
    return fn(s, "func_8002D780", d780)


def main():
    apply = "apply" in sys.argv[1:]
    out = ([a[4:] for a in sys.argv[1:] if a.startswith("out=")] or ["tmp/w2/b13"])[0]
    for p, g in FILES:
        B2.BASE = BASE
        s = sweep(g(B2.show(p)))
        dst = p if apply else out + "/" + p
        os.makedirs(os.path.dirname(dst), exist_ok=True)
        open(dst, "w", encoding="utf-8", newline=NL).write(s)
    print("w2b13 wrote %d files to %s" % (len(FILES), "the tree" if apply else out))


if __name__ == "__main__":
    main()
