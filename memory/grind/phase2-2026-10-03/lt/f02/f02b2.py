#!/usr/bin/env python3
# F02 batch 2 (prep): func_8002D320 / func_8002D780 / func_8002D518 / func_8002CA8C against v3
# (Unk1F8002B8Rec with Vec3i32 unkA8 / unkB8 / unkC8, s32 unkB4 / unkC4 / unkD4).
# Base: tmp/p2/lt/f02/17AFC.b1c.c / game.b1c.h (= the staged batch-1 v3 tree).
# usage: f02b2.py [opt=NAME]... [measure]
#   opt=d780param   D780 keeps `u8 *obj` and converts once (instead of a typed param + asm byte view)
#   opt=nopos       leave the `s32 *pos` parameters as they are
import os, re, shutil, subprocess, sys
NL = chr(10)
def rd(p): return open(p, encoding="utf-8").read()
OPT = set(a[4:] for a in sys.argv[1:] if a.startswith("opt="))

def span(src, fn):
    m = [x for x in re.finditer(r"^[a-zA-Z][^\n]*\b%s\(" % fn, src, re.M)
         if not src[x.start():src.index(NL, x.start())].rstrip().endswith(";")][0]
    i = m.start(); j = src.index(NL + "}" + NL, i) + 3
    return i, j

def edit(src, fn, reps):
    i, j = span(src, fn)
    b = src[i:j]
    for a, c, *n in reps:
        n = n[0] if n else 1
        assert b.count(a) == n, (fn, a[:90], b.count(a), n)
        b = b.replace(a, c)
    return src[:i] + b + src[j:]

def sub1(s, a, b, n=1):
    assert s.count(a) == n, (a[:90], s.count(a), n)
    return s.replace(a, b)

POS = "nopos" not in OPT
PT = "LeafPos *pos" if POS else "s32 *pos"
PX = ["pos->x", "pos->y", "pos->z"] if POS else ["pos[0]", "pos[1]", "pos[2]"]

def f320(s):
    R = [
        ("s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {",
         "s32 func_8002D320(s32 flag, Unk1F8002B8Rec *scr, %s, s32 threshold, s32 r_sq) {" % PT),
        ("        s32 *vin;\n        s32 *vout;\n", "        SVECTOR *vin;\n        Vec3i32 *vout;\n"),
        ("        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];\n"
         "        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];\n"
         "        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];\n"
         "        vin = (s32 *)(obj + 0xF8);\n",
         "        scr->unkF8.vx = %s - scr->unk60[0]->x;\n"
         "        scr->unkF8.vy = %s - scr->unk60[0]->y;\n"
         "        scr->unkF8.vz = %s - scr->unk60[0]->z;\n"
         "        vin = &scr->unkF8;\n" % tuple(PX)),
        ("        vout = (s32 *)(obj + 0x100);\n", "        vout = &scr->unk100[0];\n"),
        # Renames: each local named for the component it holds (unk100[0] / unkA8 / unkB8 are
        # Vec3i32; the old names called .y "z" and .z "y"). `x` is the LZC island's operand, so
        # its name stays; its note says what it holds.
        ("    {\n        s32 x;\n        s32 z;\n        s32 sp_var;\n        s32 min_y;\n        s32 max_y;\n"
         "        s32 y_low;\n        s32 y_high;\n        s32 y;\n",
         "    {\n"
         "        /* x: four values -- unk100[0].x, then x * x + y * y, then r_sq minus that, then\n"
         "         * its square root (the sphere's half-chord along z). Ruling 11. The name stays\n"
         "         * because x is the LZC island's operand below. */\n"
         "        s32 x;\n        s32 y;\n        s32 sp_var;\n        s32 min_z;\n        s32 max_z;\n"
         "        s32 az;\n        s32 bz;\n        s32 z;\n"),
        ("        x = *(s32 *)(obj + 0x100);\n", "        x = scr->unk100[0].x;\n"),
        ("        z = *(s32 *)(obj + 0x104);\n        if (z < neg_threshold || threshold < z) return 0;\n\n"
         "        x = x * x + z * z;\n",
         "        y = scr->unk100[0].y;\n        if (y < neg_threshold || threshold < y) return 0;\n\n"
         "        x = x * x + y * y;\n"),
        ("        max_y = 0;\n        min_y = 0;\n        y_low = *(s32 *)(obj + 0xB0);\n"
         "        if (y_low < 0) {\n            min_y = y_low;\n        } else if (min_y < y_low) {\n"
         "            max_y = y_low;\n        }\n        y_high = *(s32 *)(obj + 0xC0);\n"
         "        if (y_high < min_y) {\n            min_y = y_high;\n        } else if (max_y < y_high) {\n"
         "            max_y = y_high;\n        }\n        y = *(s32 *)(obj + 0x108);\n"
         "        if (max_y < y - x) return 0;\n        if (y + x < min_y) {\n",
         "        max_z = 0;\n        min_z = 0;\n        az = scr->unkA8.z;\n"
         "        if (az < 0) {\n            min_z = az;\n        } else if (min_z < az) {\n"
         "            max_z = az;\n        }\n        bz = scr->unkB8.z;\n"
         "        if (bz < min_z) {\n            min_z = bz;\n        } else if (max_z < bz) {\n"
         "            max_z = bz;\n        }\n        z = scr->unk100[0].z;\n"
         "        if (max_z < z - x) return 0;\n        if (z + x < min_z) {\n"),
    ]
    return edit(s, "func_8002D320", R)

def f780(s):
    if "d780param" in OPT:
        head = [("s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {\n"
                 "    if (flag == 0) {\n",
                 "s32 func_8002D780(s32 flag, u8 *obj, %s, s32 threshold, s32 r_sq) {\n"
                 "    Unk1F8002B8Rec *scr = (Unk1F8002B8Rec *)obj;\n\n"
                 "    if (flag == 0) {\n" % PT)]
    else:
        head = [("s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {\n"
                 "    if (flag == 0) {\n",
                 "s32 func_8002D780(s32 flag, Unk1F8002B8Rec *scr, %s, s32 threshold, s32 r_sq) {\n"
                 "    /* obj: the record's bytes, for the two gte_ApplyRotMatrix operands below, which\n"
                 "     * stay as written (canonical GTE island text). */\n"
                 "    u8 *obj = (u8 *)scr;\n\n"
                 "    if (flag == 0) {\n" % PT)]
    R = head + [
        ("        *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];\n"
         "        *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];\n"
         "        *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];\n",
         "        scr->unkF8.vx = %s - scr->unk60[0]->x;\n"
         "        scr->unkF8.vy = %s - scr->unk60[0]->y;\n"
         "        scr->unkF8.vz = %s - scr->unk60[0]->z;\n" % tuple(PX)),
        ("    {\n        s32 y = *(s32 *)(obj + 0x108);\n        if (y < -threshold || threshold < y) return 0;\n    }\n",
         "    {\n        s32 z = scr->unk100[0].z;\n        if (z < -threshold || threshold < z) return 0;\n    }\n"),
        ("        s32 x0 = *(s32 *)(obj + 0xA8);\n        s32 x2 = *(s32 *)(obj + 0xB8);\n"
         "        s32 z0 = *(s32 *)(obj + 0xAC);\n        s32 z2 = *(s32 *)(obj + 0xBC);\n"
         "        s32 cx = (x0 + x2) / 3;\n        s32 cz = (z0 + z2) / 3;\n"
         "        s32 px = *(s32 *)(obj + 0x100);\n        s32 pz = *(s32 *)(obj + 0x104);\n",
         "        s32 x0 = scr->unkA8.x;\n        s32 x2 = scr->unkB8.x;\n"
         "        s32 y0 = scr->unkA8.y;\n        s32 y2 = scr->unkB8.y;\n"
         "        s32 cx = (x0 + x2) / 3;\n        s32 cy = (y0 + y2) / 3;\n"
         "        s32 px = scr->unk100[0].x;\n        s32 py = scr->unk100[0].y;\n"),
        ("the 2-D cross product of one edge of the triangle (0,0), (x0,z0), (x2,z2) with the",
         "the 2-D cross product of one edge of the triangle (0,0), (x0,y0), (x2,y2) with the"),
        ("         * the edge (0,0)-(x0,z0), then (0,0)-(x2,z2), then (x0,z0)-(x2,z2). */\n"
         "        s32 cross_center = z0 * cx - x0 * cz;\n        s32 cross_point = z0 * px - x0 * pz;\n",
         "         * the edge (0,0)-(x0,y0), then (0,0)-(x2,y2), then (x0,y0)-(x2,y2). */\n"
         "        s32 cross_center = y0 * cx - x0 * cy;\n        s32 cross_point = y0 * px - x0 * py;\n"),
        ("            cross_center = z2 * cx - x2 * cz;\n            cross_point = z2 * px - x2 * pz;\n",
         "            cross_center = y2 * cx - x2 * cy;\n            cross_point = y2 * px - x2 * py;\n"),
        ("                s32 az;\n                s32 bz;\n", "                s32 ay;\n                s32 by;\n"),
        ("                /* FAKE: the third edge test's edge difference z2 - z0 is staged through the",
         "                /* FAKE: the third edge test's edge difference y2 - y0 is staged through the"),
        ("                 * a block-local dz ties dx in qty_compare_1 and takes $v1 itself. */\n"
         "                flag = z2 - z0;\n                dx = x2 - x0;\n                az = cz - z0;\n"
         "                cross_center = (flag * ax) - (dx * az);\n                bz = pz - z0;\n"
         "                cross_point = (flag * (px - x0)) - (dx * bz);\n",
         "                 * a block-local dy ties dx in qty_compare_1 and takes $v1 itself. */\n"
         "                flag = y2 - y0;\n                dx = x2 - x0;\n                ay = cy - y0;\n"
         "                cross_center = (flag * ax) - (dx * ay);\n                by = py - y0;\n"
         "                cross_point = (flag * (px - x0)) - (dx * by);\n"),
        ("        s32 y = *(s32 *)(obj + 0x108);\n        s32 sp_var;\n        s32 dist = r_sq - y * y;\n",
         "        s32 z = scr->unk100[0].z;\n        s32 sp_var;\n        s32 dist = r_sq - z * z;\n"),
        ("        s32 *p118;\n        s32 *p124;\n        s32 *p10C;\n",
         "        Vec3i32 *p118;\n        Vec3i32 *p124;\n        Vec3i32 *p10C;\n"),
        ("        p118 = (s32 *)(obj + 0x118);\n        p124 = (s32 *)(obj + 0x124);\n"
         "        p118[0] = *(s32 *)(obj + 0xA8) - *(s32 *)(obj + 0x100);\n"
         "        p118[1] = *(s32 *)(obj + 0xAC) - *(s32 *)(obj + 0x104);\n"
         "        p124[0] = *(s32 *)(obj + 0xB8) - *(s32 *)(obj + 0x100);\n"
         "        p124[1] = *(s32 *)(obj + 0xBC) - *(s32 *)(obj + 0x104);\n",
         "        p118 = &scr->unk118[0];\n        p124 = &scr->unk118[1];\n"
         "        p118->x = scr->unkA8.x - scr->unk100[0].x;\n"
         "        p118->y = scr->unkA8.y - scr->unk100[0].y;\n"
         "        p124->x = scr->unkB8.x - scr->unk100[0].x;\n"
         "        p124->y = scr->unkB8.y - scr->unk100[0].y;\n"),
        ("        p10C = (s32 *)(obj + 0x10C);\n        p10C[0] = -*(s32 *)(obj + 0x100);\n"
         "        p10C[1] = -*(s32 *)(obj + 0x104);\n",
         "        p10C = &scr->unk100[1];\n        p10C->x = -scr->unk100[0].x;\n"
         "        p10C->y = -scr->unk100[0].y;\n"),
    ]
    s = edit(s, "func_8002D780", R)
    # `dist` is r_sq - z * z: the squared radius of the sphere's cut circle in the z = 0 plane
    # (func_8002D518 takes it as r_sq, its square root as threshold), not a distance.
    i, j = span(s, "func_8002D780")
    b = s[i:j]
    assert len(re.findall(r"\bdist\b", b)) == 13 and "cut_r_sq" not in b
    return s[:i] + re.sub(r"\bdist\b", "cut_r_sq", b) + s[j:]

def f518(s):
    R = [
        ("s32 func_8002D518(s32 threshold, s32 r_sq, s32 *p1, s32 *p2) {\n    s32 x1, z1, x2, z2;\n\n"
         "    x1 = p1[0];\n    x2 = p2[0];\n",
         "s32 func_8002D518(s32 threshold, s32 r_sq, Vec3i32 *p1, Vec3i32 *p2) {\n    s32 x1, y1, x2, y2;\n\n"
         "    x1 = p1->x;\n    x2 = p2->x;\n"),
        ("        if (threshold >= x1) goto z_check;\n", "        if (threshold >= x1) goto y_check;\n"),
        ("z_check:\n    z1 = p1[1];\n    z2 = p2[1];\n\n    if (z1 < z2) {\n        if (z2 < -threshold) return 0;\n"
         "        if (threshold >= z1) goto dist_calc;\n        return 0;\n    } else {\n"
         "        if (z1 < -threshold) return 0;\n        if (threshold < z2) return 0;\n    }\n",
         "y_check:\n    y1 = p1->y;\n    y2 = p2->y;\n\n    if (y1 < y2) {\n        if (y2 < -threshold) return 0;\n"
         "        if (threshold >= y1) goto dist_calc;\n        return 0;\n    } else {\n"
         "        if (y1 < -threshold) return 0;\n        if (threshold < y2) return 0;\n    }\n"),
        ("        s32 ax = p2[0] - p1[0];\n        s32 x1r = p1[0];\n        s32 az = p2[1] - p1[1];\n"
         "        s32 z1r = p1[1];\n\n        s32 ax_sq = ax * ax;\n        s32 az_sq = az * az;\n"
         "        s32 cx = ax * x1r;\n        s32 cz = az * z1r;\n        s32 x1_sq = x1r * x1r;\n"
         "        s32 z1_sq = z1r * z1r;\n",
         "        s32 ax = p2->x - p1->x;\n        s32 x1r = p1->x;\n        s32 ay = p2->y - p1->y;\n"
         "        s32 y1r = p1->y;\n\n        s32 ax_sq = ax * ax;\n        s32 ay_sq = ay * ay;\n"
         "        s32 cx = ax * x1r;\n        s32 cy = ay * y1r;\n        s32 x1_sq = x1r * x1r;\n"
         "        s32 y1_sq = y1r * y1r;\n"),
        ("        s32 dot2 = (cx + cz) * 2;\n", "        s32 dot2 = (cx + cy) * 2;\n"),
        ("        s32 c_val = ((x1_sq + z1_sq) - r_sq) >> 9;\n        s32 dist_sq = ax_sq + az_sq;\n",
         "        s32 c_val = ((x1_sq + y1_sq) - r_sq) >> 9;\n        s32 dist_sq = ax_sq + ay_sq;\n"),
    ]
    s = edit(s, "func_8002D518", R)
    return sub1(s, "/* func_8002D518 - segment/circle test in x/z. Returns 0 when p1 and p2 lie\n"
                   " * on the same side outside the +-threshold band in x or in z; otherwise solves",
                   "/* func_8002D518 - segment/circle test in x/y. Returns 0 when p1 and p2 lie\n"
                   " * on the same side outside the +-threshold band in x or in y; otherwise solves")

def fca8c(s):
    R = [
        ("    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8;\n"),
        ("        if (*(s32 *)(scr + 0x84) < x - r || x + r < *(s32 *)(scr + 0x78)) {\n",
         "        if (scr->unk84.x < x - r || x + r < scr->unk78.x) {\n"),
        ("            if (*(s32 *)(scr + 0x88) < y - r || y + r < *(s32 *)(scr + 0x7C)) {\n",
         "            if (scr->unk84.y < y - r || y + r < scr->unk78.y) {\n"),
        ("                if (*(s32 *)(scr + 0x8C) < z - r || z + r < *(s32 *)(scr + 0x80)) {\n",
         "                if (scr->unk84.z < z - r || z + r < scr->unk78.z) {\n"),
        ("    *(s32 *)(scr + 0xB4) = seenMask;\n    *(s32 *)(scr + 0xC4) = hitMask;\n",
         "    scr->unkB4 = seenMask;\n    scr->unkC4 = hitMask;\n"),
    ]
    if POS:
        R += [("(s32 *)&SPAD->unkA8[id][i],", "&SPAD->unkA8[id][i],", 2),
              ("scr, (s32 *)0,", "scr, NULL,", 2)]
    return edit(s, "func_8002CA8C", R)

def build():
    src = rd("tmp/p2/lt/f02/17AFC.b1c.c")
    g = rd("tmp/p2/lt/f02/game.b1c.h")
    src = f320(src); src = f780(src); src = f518(src); src = fca8c(src)
    D780P = ("u8 *obj" if "d780param" in OPT else "Unk1F8002B8Rec *scr")
    src = sub1(src,
        "extern s32 func_8002D320(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);\n"
        "extern s32 func_8002D780(s32 flag, u8 *obj, s32 *pos, s32 threshold, s32 r_sq);\n",
        "extern s32 func_8002D320(s32 flag, Unk1F8002B8Rec *scr, %s, s32 threshold, s32 r_sq);\n"
        "extern s32 func_8002D780(s32 flag, %s, %s, s32 threshold, s32 r_sq);\n" % (PT, D780P, PT))
    g = sub1(g, " * address (`scr`) and pass it as `obj` to func_8002E838 / func_8002EA24 / func_8002D320 /\n",
                " * address (`scr`) and pass it to func_8002E838 / func_8002EA24 / func_8002D320 /\n")
    return src, g

if __name__ == "__main__":
    src, g = build()
    tag = "b2" + "".join("_" + o for o in sorted(OPT))
    open("tmp/p2/lt/f02/v/17AFC.%s.c" % tag, "w", encoding="utf-8", newline=NL).write(src)
    open("tmp/p2/lt/f02/v/game.h.%s" % tag, "w", encoding="utf-8", newline=NL).write(g)
    if "measure" in sys.argv[1:]:
        shutil.rmtree("tmp/p2/wk", ignore_errors=True)
        os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
        shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
        open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
        open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)
        fs = ["func_8002D320", "func_8002D780", "func_8002D518", "func_8002CA8C"]
        r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/17AFC"] + fs, capture_output=True, text=True)
        print(r.stdout + r.stderr)
        t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/17AFC"], capture_output=True, text=True)
        print(t.stdout + t.stderr)
    print("wrote", tag)
