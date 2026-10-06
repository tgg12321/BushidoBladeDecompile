#!/usr/bin/env python3
# Ablations for w2b8 (run from the repo root with the batch applied): the carried FAKEs of the moved bodies
# without a measured score, the in-place sweep of their holders, and the casts on the edited lines (B6).
# Writes tmp/w2/abl8/<name>.c + list.txt for ../f08/run_abl_w2b3.ps1 -List tmp/w2/abl8/list.txt.
import os, re, sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "fzz"))
from abl_w2b5 import body, sub, inline, drop_fake

F2B, F9F = "src/main/2B344.c", "src/main/9F9C.c"
A = []

# ---- carried FAKEs
A.append(("a820_pair", "func_8001A820", F9F, lambda b: sub(drop_fake(b, "/* FAKE: cancellation pair"),
    "            hi++;\n            hi--;\n", "")))
Q63 = ("                p = &D_800A37D2;\n", "                p[t != 0]++;\n")


def c8dc_p(b):
    b = re.sub(r" +/\* FAKE: (second handle|indexes past)[^*]*(?:\*(?!/)[^*]*)*\*/\n", "", b)
    b = sub(sub(b, Q63[0], "", 2), Q63[1], "                (&D_800A37D2)[t != 0]++;\n", 2)
    return sub(b, "            u8 *p;\n", "", 2)


def c8dc_sym(b):
    b = re.sub(r" +/\* FAKE: (second handle|indexes past)[^*]*(?:\*(?!/)[^*]*)*\*/\n", "", b)
    b = sub(sub(b, Q63[0], "", 2), Q63[1],
            "                if (t != 0) {\n                    D_800A37D3++;\n                } else {\n"
            "                    D_800A37D2++;\n                }\n", 2)
    return sub(b, "            u8 *p;\n", "", 2)


F17, F368 = "src/main/17AFC.c", "src/main/368E4.c"
A.append(("bcf0_copy", "func_8001BCF0", F9F, lambda b: sub(drop_fake(b, "/* FAKE: block copy of unk_B8"),
    "    D_800F6608.unk_00 = *(Vec3i32 *)&arg0->unk_B8;\n",
    "    D_800F6608.unk_00.x = arg0->unk_B8.vx;\n    D_800F6608.unk_00.y = arg0->unk_B8.vy;\n    D_800F6608.unk_00.z = arg0->unk_B8.vz;\n")))
A.append(("bcf0_diff", "func_8001BCF0", F9F, lambda b: inline(drop_fake(b, "/* FAKE: diff computed"),
    "    s32 diff = 0x1000 - arg1;\n", "diff", "0x1000 - arg1")))
A.append(("bcf0_div4", "func_8001BCF0", F9F, lambda b: inline(drop_fake(b, "/* FAKE: div4 taken"),
    "        s32 div4 = arg1 / 4;\n", "div4", "arg1 / 4")))
A.append(("bcf0_lhu", "func_8001BCF0", F9F, lambda b: inline(drop_fake(b, "/* FAKE: unk_1C8.vy read"),
    "        u16 lhu_val = arg0->unk_1C8.vy;\n", "lhu_val", "(u16)arg0->unk_1C8.vy")))
A.append(("bcf0_val", "func_8001BCF0", F9F, lambda b: inline(drop_fake(b, "/* FAKE: val holds"),
    "        s32 val;\n", "val", "0xB00 - div4", "        val = 0xB00 - div4;\n")))
A.append(("e404_pad", "func_8001E404", F9F, lambda b: sub(drop_fake(b, "/* FAKE: frame layout"), "    volatile u32 pre_pad[2];\n", "")))
A.append(("e6e4_pad", "func_8001E6E4", F9F, lambda b: sub(drop_fake(b, "/* FAKE: frame layout"), "    volatile u32 pre_pad[2];\n", "")))
A.append(("bf4_count1", "func_80046BF4", F368, lambda b: sub(sub(sub(drop_fake(b, "/* FAKE: count1"), "    u16 count1;\n", ""),
    "        count1 = D_800A38D6 + 1;\n", ""), "        D_800A38D6 = count1;\n", "        D_800A38D6 = D_800A38D6 + 1;\n")))
A.append(("25e0_v0m", "func_800325E0", F17, lambda b: sub(sub(sub(drop_fake(b, "/* FAKE: v0_m holds"),
    "                u32 v0_m = (u32)-2;\n", ""), "                v0_m &= sp_tmp;\n", ""),
    "                v1_m = 0x16 - v0_m;\n", "                v1_m = 0x16 - (sp_tmp & ~1);\n")))
A.append(("c8dc_p", "func_8001C8DC", F9F, c8dc_p))
A.append(("c8dc_sym", "func_8001C8DC", F9F, c8dc_sym))

# ---- sweep
A.append(("b294_t", "func_8001B294", F9F, lambda b: sub(b,
    "    {\n        s32 t1 = a0->unk_F4.z;\n        s32 t2 = a1->unk_F4.z;\n        D_800F6608.unk_10.vx = 0;\n"
    "        D_800F6608.unk_00.z = (t1 + t2) / 2;\n    }\n",
    "    D_800F6608.unk_10.vx = 0;\n    D_800F6608.unk_00.z = (a0->unk_F4.z + a1->unk_F4.z) / 2;\n")))
A.append(("b294_d", "func_8001B294", F9F, lambda b: sub(b,
    "    {\n        s32 dx = a1->unk_F4.x - a0->unk_F4.x;\n        s32 dz = a1->unk_F4.z - a0->unk_F4.z;\n"
    "        v0 = ratan2(dx, dz);\n    }\n",
    "    v0 = ratan2(a1->unk_F4.x - a0->unk_F4.x, a1->unk_F4.z - a0->unk_F4.z);\n")))
A.append(("b294_v0", "func_8001B294", F9F, lambda b: sub(sub(sub(b, "    s32 v0;\n", ""),
    "        v0 = ratan2(dx, dz);\n    }\n", "        D_800F6608.unk_10.vy = 0x400 - ratan2(dx, dz);\n    }\n"),
    "    D_800F6608.unk_10.vy = 0x400 - v0;\n", "")))
A.append(("b294_dv", "func_8001B294", F9F, lambda b: sub(sub(sub(b, "    s32 v0;\n", ""),
    "    {\n        s32 dx = a1->unk_F4.x - a0->unk_F4.x;\n        s32 dz = a1->unk_F4.z - a0->unk_F4.z;\n"
    "        v0 = ratan2(dx, dz);\n    }\n", ""),
    "    D_800F6608.unk_10.vy = 0x400 - v0;\n",
    "    D_800F6608.unk_10.vy = 0x400 - ratan2(a1->unk_F4.x - a0->unk_F4.x, a1->unk_F4.z - a0->unk_F4.z);\n")))
A.append(("b478_s2", "func_8001B478", F9F, lambda b: re.sub(r"\bs2->", "D_800F5328.",
    sub(b, "    Rec44 *s2 = &D_800F5328;\n", ""))))
A.append(("b478_z", "func_8001B478", F9F, lambda b: sub(b, "        D_800F5328.unk_00.z = ", "        s2->unk_00.z = ")))
A.append(("b478_cnt", "func_8001B478", F9F, lambda b: sub(b,
    "            {\n                s16 cnt = counter - 1;\n                s2->unk_10.vy = old12 + (diff >> 2);\n"
    "                D_800A36FC = cnt;\n            }\n",
    "            s2->unk_10.vy = old12 + (diff >> 2);\n            D_800A36FC = counter - 1;\n")))
A.append(("b478_res", "func_8001B478", F9F, lambda b: sub(b,
    "            {\n                s32 base_val = obj->other->unk_F4.y;\n"
    "                s32 result = ratan2(base_val - a2, D_800A387C);\n"
    "                val = (result * (0x400 - val)) >> 10;\n            }\n",
    "            val = (ratan2(obj->other->unk_F4.y - a2, D_800A387C) * (0x400 - val)) >> 10;\n")))
A.append(("b748_zval", "func_8001B748", F9F, lambda b: sub(sub(sub(b, "    s32 zval;\n", ""),
    "        zval = (frac * (a->h8)) + (inv_frac * (b->h8));\n", ""),
    "        dst->unk_00.z = zval >> 12;\n", "        dst->unk_00.z = ((frac * (a->h8)) + (inv_frac * (b->h8))) >> 12;\n")))
def b748_v(b):
    c = "(s16)" if "val - (s16)dst->unk_10.vy" in b else ""
    b = sub(sub(b, "    s32 v;\n", ""), "    v = math_SignExt12Div(val - %sdst->unk_10.vy, 0x10);\n" % c, "")
    return sub(b, "    dst->unk_10.vy = dst->unk_10.vy + v;\n",
               "    dst->unk_10.vy = dst->unk_10.vy + math_SignExt12Div(val - %sdst->unk_10.vy, 0x10);\n" % c)


A.append(("b748_v", "func_8001B748", F9F, b748_v))
A.append(("b748_cast", "func_8001B748", F9F, lambda b: sub(b, "(s16)dst->unk_10.", "dst->unk_10.", 2)))
A.append(("b748_avg", "func_8001B748", F9F, lambda b: sub(sub(b,
    "        s32 avg = ((s32) (sum + (((u32) sum) >> 31))) >> 1;\n", ""),
    "        if ((avg - base->unk_180.y) < 0xC8) {\n",
    "        if ((((s32) (sum + (((u32) sum) >> 31))) >> 1) - base->unk_180.y < 0xC8) {\n")))
A.append(("b748_half", "func_8001B748", F9F, lambda b: sub(b,
    "        if ((((s32) (sum + (((u32) sum) >> 31))) >> 1) - base->unk_180.y < 0xC8) {\n",
    "        if (sum / 2 - base->unk_180.y < 0xC8) {\n")))
A.append(("b748_sum", "func_8001B748", F9F, lambda b: sub(sub(b,
    "        s32 sum = base->unk_198[0].y + base->unk_198[1].y;\n", ""),
    "        if ((((s32) (sum + (((u32) sum) >> 31))) >> 1) - base->unk_180.y < 0xC8) {\n",
    "        if ((base->unk_198[0].y + base->unk_198[1].y) / 2 - base->unk_180.y < 0xC8) {\n")))
A.append(("2408_t", "func_80022408", F9F, lambda b: sub(sub(sub(sub(sub(sub(b, "    s32 t1;\n", ""), "    s32 t2;\n", ""),
    "    t1 = arg0->x;\n", ""), "    t2 = arg0->z;\n", ""), "/ 2) - t1;", "/ 2) - arg0->x;"), "/ 2) - t2;", "/ 2) - arg0->z;")))
A.append(("e404_cast", "func_8001E404", F9F, lambda b: re.sub(r"\(u16\)(D_800FF5D[8AC])", r"\1", b)))
A.append(("e6e4_cast", "func_8001E6E4", F9F, lambda b: re.sub(r"\(u16\)(D_800FF5D[8AC])", r"\1", b)))
A.append(("c9a4_cast", "func_8003C9A4", F2B, lambda b: sub(b, "(s16)(D_800A36AC << 2)", "D_800A36AC << 2")))
A.append(("cd10_cast", "func_8003CD10", F2B, lambda b: sub(b, "(s16)(D_800A36AC << 2)", "D_800A36AC << 2")))
A.append(("ce18_v0", "func_8003CE18", F2B, lambda b: sub(sub(sub(b, "    s32 v0;\n", ""),
    "    v0 = math_FovToScreenDist(0x2D);\n", ""), "    SetGeomScreen(v0);\n", "    SetGeomScreen(math_FovToScreenDist(0x2D));\n")))
A.append(("ce18_val", "func_8003CE18", F2B, lambda b: sub(sub(b,
    "        u16 val = (u16)D_80101EC8[player].unk_0E;\n", ""),
    "        if ((u16)(val - 6) < 2) {\n", "        if ((u16)((u16)D_80101EC8[player].unk_0E - 6) < 2) {\n")))


# ---- second sweep pass (reused multi-role locals, a single-use table read, the cast left on ce18's line)

def b748_roles(b):
    """cur / t reused for x, y, z, the pitch and w18: one local per role"""
    marks = ["    cur = dst->unk_00.x;\n", "    cur = dst->unk_00.y;\n", "    cur = dst->unk_00.z;\n",
             "    if (use_high) {\n        t = ((frac_s1 * 0x180)", "    if (use_high) {\n        t = ((frac_s1 * 0x7D0)"]
    idx = [b.index(m) for m in marks]
    end = b.index("    dst->h30[0][0] = 0x64;\n", idx[-1])
    names = ["x", "y", "z", "p", "w"]
    out = b[:idx[0]]
    for k in range(5):
        seg = b[idx[k]:(idx[k + 1] if k < 4 else end)]
        seg = re.sub(r"\bcur\b", "cur_" + names[k], seg)
        seg = re.sub(r"\bt\b", "t_" + names[k], seg)
        out += seg
    out += b[end:]
    out = sub(out, "    s32 cur;\n", "    s32 cur_x, cur_y, cur_z, cur_w;\n")
    return sub(out, "    s32 t;\n", "    s32 t_x, t_y, t_z, t_p, t_w;\n")


def a820_tbl(b):
    return sub(sub(b, "            s32 tbl = g_sqrt_table_u8[dist_sq >> sh];\n", ""),
               "            dist = (u32)(tbl << 16) >> (0x13 - ((u32)sh >> 1));\n",
               "            dist = (u32)(g_sqrt_table_u8[dist_sq >> sh] << 16) >> (0x13 - ((u32)sh >> 1));\n")


def ce18_val2(b):
    """after ce18_val: the inner (u16) (the subtraction's low 16 bits are the same either way)"""
    return sub(b, "        if ((u16)((u16)D_80101EC8[player].unk_0E - 6) < 2) {\n",
               "        if ((u16)(D_80101EC8[player].unk_0E - 6) < 2) {\n")


A.append(("b748_roles", "func_8001B748", F9F, b748_roles))
A.append(("a820_tbl", "func_8001A820", F9F, a820_tbl))
A.append(("ce18_val2", "func_8003CE18", F2B, ce18_val2))

# the labels w2b8.py adds: stripped first when re-measuring on the landed batch
PRE = {"b294_t": "t1 / t2 read", "b478_s2": "s2 holds", "b478_cnt": "cnt computed", "b748_zval": "zval's products",
       "b748_v": "v computed", "b748_roles": "cur / t are reused"}


def unlabel(b, start):
    return drop_fake(b, "/* FAKE: " + start)


def main():
    only = set(sys.argv[1:])
    os.makedirs("tmp/w2/abl8", exist_ok=True)
    rows = []
    for name, fn, path, t in A:
        if only and name not in only:
            continue
        p = "tmp/w2/abl8/%s.c" % name
        b0 = body(path, fn)
        try:
            b1 = t(unlabel(b0, PRE[name]) if name in PRE and PRE[name] in b0 else b0)
        except SystemExit as e:
            print("SKIP %s: %s" % (name, e))
            continue
        if b1 == b0:
            print("SKIP %s: no change" % name)
            continue
        open(p, "w", encoding="utf-8", newline="\n").write(b1)
        rows.append("%s %s %s" % (name, fn, p))
    open("tmp/w2/abl8/list.txt", "w", newline="\n").write("\n".join(rows) + "\n")
    print(len(rows), "candidates")


if __name__ == "__main__":
    main()
