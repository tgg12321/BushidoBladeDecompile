#!/usr/bin/env python3
# Ablations for w2b13 (run from the repo root with the batch applied): the carried FAKEs and the B4 sweep of the
# three Q115 bodies. Writes tmp/w2/abl13/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl13/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, drop_fake

F = "src/main/17AFC.c"
A = []
TBL = "s32 tbl = g_sqrt_table_u8[(u32)%s >> shift];\n"


def no_tbl(b, var, ind):
    return sub(sub(b, ind + TBL % var, ""), ind + "%s = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n"
               % ("len" if var != "dist_sq" else "dist"),
               ind + "%s = (u32)(g_sqrt_table_u8[(u32)%s >> shift] << 16) >> (0x13 - ((u32)shift >> 1));\n"
               % ("len" if var != "dist_sq" else "dist", var))


# ---- func_8002CD58
A.append(("cd58_tbl1", "func_8002CD58", lambda b: sub(b, "                s32 tbl = g_sqrt_table_u8[(u32)len_sq >> shift];\n                len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n", "                len = (u32)(g_sqrt_table_u8[(u32)len_sq >> shift] << 16) >> (0x13 - ((u32)shift >> 1));\n")))
A.append(("cd58_tbl2", "func_8002CD58", lambda b: no_tbl(b, "xz_sq", "                    ")))
A.append(("cd58_yaw_down", "func_8002CD58", lambda b: sub(sub(sub(b, "    s32 yaw;\n", ""),
    "            yaw = ratan2(obj->unkA8.x, obj->unkA8.z);\n", ""),
    "            obj->unkF8.vy = 0x800 - yaw;\n", "            obj->unkF8.vy = 0x800 - ratan2(obj->unkA8.x, obj->unkA8.z);\n")))
A.append(("cd58_yaw_up", "func_8002CD58", lambda b: sub(sub(sub(b, "    s32 yaw;\n", ""),
    "            yaw = ratan2(obj->unkA8.x, obj->unkA8.z);\n",
    "            obj->unkF8.vy = 0x800 - ratan2(obj->unkA8.x, obj->unkA8.z);\n"),
    "            obj->unkF8.vy = 0x800 - yaw;\n", "")))
A.append(("cd58_pitch", "func_8002CD58", lambda b: sub(sub(sub(b, "    s32 pitch;\n", ""),
    "            pitch = ratan2(obj->unkA8.y, len);\n            obj->unkF8.vx = 0x800 - pitch;\n",
    "            obj->unkF8.vx = 0x800 - ratan2(obj->unkA8.y, len);\n"), "\n\n\n", "\n\n\n", 0) if False else
    sub(sub(b, "    s32 pitch;\n", ""), "            pitch = ratan2(obj->unkA8.y, len);\n            obj->unkF8.vx = 0x800 - pitch;\n",
        "            obj->unkF8.vx = 0x800 - ratan2(obj->unkA8.y, len);\n")))
A.append(("cd58_npitch", "func_8002CD58", lambda b: sub(sub(b, "    s32 npitch;\n", ""),
    "    npitch = ratan2(obj->unkC8.y, nxz_len);\n    obj->unkF8.vx = 0x800 - npitch;\n",
    "    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, nxz_len);\n")))


def cd58_temp_split(b):
    # temp's second role (the table byte) gets its own block local
    return sub(sub(b, "            temp = g_sqrt_table_u8[(u32)temp >> shift];\n            nxz_len = (u32)(temp << 16) >> (0x13 - ((u32)shift >> 1));\n",
                   "            s32 tbl = g_sqrt_table_u8[(u32)temp >> shift];\n            nxz_len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n"),
               "            s32 shift = 0x16 - (lzcr & ~1);\n            s32 tbl", "            s32 shift = 0x16 - (lzcr & ~1);\n            s32 tbl")


A.append(("cd58_temp_split", "func_8002CD58", cd58_temp_split))


def cd58_len_split(b):
    # len's second role (|a.xz|) gets its own local
    b = sub(b, "        s32 len;\n", "        s32 len;\n        s32 len_xz;\n")
    b = sub(b, "                len = (u32)g_sqrt_table_u8[xz_sq] >> 3;\n", "                len_xz = (u32)g_sqrt_table_u8[xz_sq] >> 3;\n")
    b = sub(b, "                    len = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n",
            "                    len_xz = (u32)(tbl << 16) >> (0x13 - ((u32)shift >> 1));\n")
    return sub(b, "            pitch = ratan2(obj->unkA8.y, len);\n", "            pitch = ratan2(obj->unkA8.y, len_xz);\n")


A.append(("cd58_len_split", "func_8002CD58", cd58_len_split))
# ---- func_8002DAD0
A.append(("dad0_dist", "func_8002DAD0", lambda b: sub(drop_fake(b, "/* FAKE: the scaled Z delta"),
    "    dist = obj->unkC8.z;\n    dist >>= 6;\n    dist_sq = obj->unkC8.x * obj->unkC8.x + dist * dist;\n    obj->unkC8.z = dist;\n",
    "    {\n        s32 dz = obj->unkC8.z;\n        dz >>= 6;\n        dist_sq = obj->unkC8.x * obj->unkC8.x + dz * dz;\n        obj->unkC8.z = dz;\n    }\n")))
A.append(("dad0_tbl", "func_8002DAD0", lambda b: no_tbl(b, "dist_sq", "            ")))
A.append(("dad0_angle2_down", "func_8002DAD0", lambda b: sub(sub(sub(b, "    s32 angle2;\n", ""),
    "    angle2 = ratan2(obj->unkC8.y, dist);\n", ""),
    "    obj->unkF8.vx = 0x800 - angle2;\n", "    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, dist);\n")))
A.append(("dad0_angle2_up", "func_8002DAD0", lambda b: sub(sub(sub(b, "    s32 angle2;\n", ""),
    "    angle2 = ratan2(obj->unkC8.y, dist);\n", "    obj->unkF8.vx = 0x800 - ratan2(obj->unkC8.y, dist);\n"),
    "    obj->unkF8.vx = 0x800 - angle2;\n", "")))
# ---- func_8002D780
A.append(("d780_flag", "func_8002D780", lambda b: sub(sub(sub(drop_fake(b, "/* FAKE: the third edge test's"),
    "                s32 bz;\n", "                s32 bz;\n                s32 dz;\n"),
    "                flag = z2 - z0;\n", "                dz = z2 - z0;\n"),
    "flag * ", "dz * ", 2)))
A.append(("d780_m", "func_8002D780", lambda b: sub(drop_fake(b, "/* FAKE: same-value re-store"), "                m = dist;\n", "")))
A.append(("d780_ax", "func_8002D780", lambda b: sub(sub(b, "                s32 ax = cx - x0;\n", ""),
    "(flag * ax)", "(flag * (cx - x0))")))
A.append(("d780_az", "func_8002D780", lambda b: sub(sub(sub(b, "                s32 az;\n", ""), "                az = cz - z0;\n", ""),
    "(dx * az)", "(dx * (cz - z0))")))
A.append(("d780_bz", "func_8002D780", lambda b: sub(sub(sub(b, "                s32 bz;\n", ""), "                bz = pz - z0;\n", ""),
    "(dx * bz)", "(dx * (pz - z0))")))


def d780_cross(b):
    # one pair of locals per side test
    b = sub(b, "        s32 cross_center = z0 * cx - x0 * cz;\n        s32 cross_point = z0 * px - x0 * pz;\n",
            "        s32 c1 = z0 * cx - x0 * cz;\n        s32 p1 = z0 * px - x0 * pz;\n")
    b = sub(b, "        if ((cross_center ^ cross_point) >= 0) {\n            cross_center = z2 * cx - x2 * cz;\n            cross_point = z2 * px - x2 * pz;\n            if ((cross_center ^ cross_point) >= 0) {\n",
            "        if ((c1 ^ p1) >= 0) {\n            s32 c2 = z2 * cx - x2 * cz;\n            s32 p2 = z2 * px - x2 * pz;\n            if ((c2 ^ p2) >= 0) {\n")
    b = sub(b, "                cross_center = (flag * ax) - (dx * az);\n", "                s32 c3;\n                s32 p3;\n                c3 = (flag * ax) - (dx * az);\n") if False else b
    b = sub(b, "                cross_center = (flag * ax) - (dx * az);\n", "                c3 = (flag * ax) - (dx * az);\n")
    b = sub(b, "                cross_point = (flag * (px - x0)) - (dx * bz);\n                if ((cross_center ^ cross_point) >= 0)\n",
            "                p3 = (flag * (px - x0)) - (dx * bz);\n                if ((c3 ^ p3) >= 0)\n")
    return sub(b, "                s32 bz;\n", "                s32 bz;\n                s32 c3;\n                s32 p3;\n")


A.append(("d780_cross", "func_8002D780", d780_cross))

# ---- the labels re-measured on the swept bodies (the drops above change their anchors)
def cd58_len_split_f(b):
    b = sub(b, "        s32 len;\n", "        s32 len;\n        s32 len_xz;\n")
    b = sub(b, "                len = (u32)g_sqrt_table_u8[xz_sq] >> 3;\n", "                len_xz = (u32)g_sqrt_table_u8[xz_sq] >> 3;\n")
    b = sub(b, "                    len = (u32)(g_sqrt_table_u8[(u32)xz_sq >> shift] << 16)",
            "                    len_xz = (u32)(g_sqrt_table_u8[(u32)xz_sq >> shift] << 16)")
    return sub(b, "ratan2(obj->unkA8.y, len);", "ratan2(obj->unkA8.y, len_xz);")


def d780_flag_f(b):
    b = drop_fake(b, "/* FAKE: the third edge test's")
    b = sub(sub(b, "                s32 dx;\n", "                s32 dx;\n                s32 dz;\n"), "                flag = z2 - z0;\n",
            "                dz = z2 - z0;\n")
    return sub(b, "flag * ", "dz * ", 2)


def d780_cross_f(b):
    b = sub(b, "        s32 cross_center = z0 * cx - x0 * cz;\n        s32 cross_point = z0 * px - x0 * pz;\n",
            "        s32 c1 = z0 * cx - x0 * cz;\n        s32 p1 = z0 * px - x0 * pz;\n")
    b = sub(b, "        if ((cross_center ^ cross_point) >= 0) {\n            cross_center = z2 * cx - x2 * cz;\n"
               "            cross_point = z2 * px - x2 * pz;\n            if ((cross_center ^ cross_point) >= 0) {\n",
            "        if ((c1 ^ p1) >= 0) {\n            s32 c2 = z2 * cx - x2 * cz;\n            s32 p2 = z2 * px - x2 * pz;\n"
            "            if ((c2 ^ p2) >= 0) {\n")
    b = sub(b, "                s32 dx;\n", "                s32 dx;\n                s32 c3;\n                s32 p3;\n")
    b = sub(b, "                cross_center = (flag * ax)", "                c3 = (flag * ax)")
    b = sub(b, "                cross_point = (flag * (px - x0))", "                p3 = (flag * (px - x0))")
    return sub(b, "                if ((cross_center ^ cross_point) >= 0)\n", "                if ((c3 ^ p3) >= 0)\n")


A.append(("cd58_len_split_f", "func_8002CD58", cd58_len_split_f))
A.append(("d780_flag_f", "func_8002D780", d780_flag_f))
A.append(("d780_cross_f", "func_8002D780", d780_cross_f))

def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl13", exist_ok=True)
    rows = []
    for name, fn, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl13/%s.c" % name
        b0 = body(F, fn)
        try:
            b1 = t(b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        if b1 == b0:
            print("SKIP %s: no change" % name)
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl13/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
