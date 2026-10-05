#!/usr/bin/env python3
# F02 batch 3 (prep): Unk1F8002B8Rec.unk00 becomes a union of per-function views; func_8002A458 /
# func_80031B24 get theirs; func_8002EA24 / func_8002E838 / func_80031890 take the typed record.
# Base: v/17AFC.b2s.c (batch 2 applied over the staged batch 1) / game.b1c.h.
# usage: f02b3.py [opt=NAME]... [measure]
import os, re, shutil, subprocess, sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b2 as B
NL = chr(10)
OPT = set(a[4:] for a in sys.argv[1:] if a.startswith("opt="))
edit, sub1, rd = B.edit, B.sub1, B.rd

VIEWS = """/* Unk1F8002B8Rec.unk00 is per-function scratch: each function that keeps data in these 0x60 bytes
 * has its own layout of them, one member of Unk1F8002B8Unk00 each, and uses only its own. */

/* func_8002A458's layout: its segment's two end points, base then tip (func_8002AB08 writes them
 * before each call). It aims unk60[0] / unk60[1] at them for func_8002E838 / func_8002EA24. */
typedef struct {
    LeafPos unk00[2];
} Unk1F8002B8_8002A458;

/* func_80031B24's layout: one D_80106A78 object's step, prev_pos then pos. It aims unk60[0] /
 * unk60[1] at them for func_8002E838 / func_8002EA24. */
typedef struct {
    Vec3i32 unk00[2];
} Unk1F8002B8_80031B24;

/* unk00 (bytes) sizes the union to 0x60. func_8002AB08 still uses these bytes through its byte
 * pointer; func_80030D7C / func_800321E8 lay out more than unk00 (func_8005344C's work area from
 * +0x38 through +0x123) and have no member here. */
typedef union {
    u8 unk00[0x60];
    Unk1F8002B8_8002A458 v8002A458;
    Unk1F8002B8_80031B24 v80031B24;
} Unk1F8002B8Unk00;

"""

def game(g):
    g = sub1(g, "/* 17AFC's view of the last 0x148 bytes of the scratchpad,",
             VIEWS + "/* 17AFC's view of the last 0x148 bytes of the scratchpad,")
    g = sub1(g, "typedef struct {\n    u8 unk00[0x60];\n    LeafPos *unk60[3];",
             "typedef struct {\n    Unk1F8002B8Unk00 unk00;\n    LeafPos *unk60[3];")
    g = sub1(g, " * address (`scr`) and pass it as `obj` to func_8002E838 / func_8002EA24 / func_8002D320 /\n",
             " * address (`scr`) and pass it to func_8002E838 / func_8002EA24 / func_8002D320 /\n")
    g = sub1(g, " * (0,0) / unkA8 / unkB8. unk00 is used differently by func_8002A458 / func_8002AB08 /\n"
                " * func_80030D7C / func_80031B24 / func_800321E8 and is not yet typed. */\n",
             " * (0,0) / unkA8 / unkB8. unk00 is per-function scratch (Unk1F8002B8Unk00): func_8002A458 /\n"
             " * func_80031B24 use their members; func_8002AB08 / func_80030D7C / func_800321E8 still use\n"
             " * it through a byte pointer. */\n")
    return g

def fe838(s):
    s = edit(s, "func_8002E838", [
        ("void func_8002E838(u8 *obj) {\n    Unk1F8002B8Rec *scr = (Unk1F8002B8Rec *)obj;\n",
         "void func_8002E838(Unk1F8002B8Rec *scr) {\n")])
    return sub1(s, "extern void func_8002E838(u8 *obj);\n", "extern void func_8002E838(Unk1F8002B8Rec *scr);\n")

def f31890(s):
    return edit(s, "func_80031890", [
        ("void func_80031890(u8 *obj, Obj80106A78 *ent, s32 idx) {\n    Unk1F8002B8Rec *scr;\n",
         "void func_80031890(Unk1F8002B8Rec *scr, Obj80106A78 *ent, s32 idx) {\n"),
        ("    scr = (Unk1F8002B8Rec *)obj;\n", "")])

def fea24(s):
    s = edit(s, "func_8002EA24", [
        ("s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq) {\n    s32 *vin;\n    s32 *vout;\n"
         "    *(s16 *)(obj + 0xF8) = pos[0] - (*(s32 **)(obj + 0x60))[0];\n"
         "    *(s16 *)(obj + 0xFA) = pos[1] - (*(s32 **)(obj + 0x60))[1];\n"
         "    *(s16 *)(obj + 0xFC) = pos[2] - (*(s32 **)(obj + 0x60))[2];\n"
         "    vin = (s32 *)(obj + 0xF8);\n",
         "s32 func_8002EA24(Unk1F8002B8Rec *scr, LeafPos *pos, s32 threshold, s32 r_sq) {\n"
         "    SVECTOR *vin;\n    Vec3i32 *vout;\n"
         "    scr->unkF8.vx = pos->x - scr->unk60[0]->x;\n"
         "    scr->unkF8.vy = pos->y - scr->unk60[0]->y;\n"
         "    scr->unkF8.vz = pos->z - scr->unk60[0]->z;\n"
         "    vin = &scr->unkF8;\n"),
        ("    vout = (s32 *)(obj + 0x100);\n", "    vout = &scr->unk100[0];\n"),
        ("    {\n        s32 z;\n        s32 a0_var;\n        s32 sp_var;\n        s32 min_y;\n        s32 max_y;\n"
         "        s32 y_low;\n        s32 y;\n        s32 x;\n",
         "    {\n        s32 y;\n"
         "        /* a0_var: two values -- r_sq - (x * x + y * y), then its square root (the sphere's\n"
         "         * half-chord along z). Ruling 11. The name stays because a0_var is the LZC island's\n"
         "         * operand below. */\n"
         "        s32 a0_var;\n        s32 sp_var;\n        s32 min_z;\n        s32 max_z;\n"
         "        s32 az;\n        s32 z;\n        s32 x;\n"),
        ("        x = *(s32 *)(obj + 0x100);\n", "        x = scr->unk100[0].x;\n"),
        ("        z = *(s32 *)(obj + 0x104);\n        if (z < neg_threshold || threshold < z) return 0;\n\n"
         "        sq = x * x + z * z;\n",
         "        y = scr->unk100[0].y;\n        if (y < neg_threshold || threshold < y) return 0;\n\n"
         "        sq = x * x + y * y;\n"),
        ("        max_y = 0;\n        min_y = 0;\n        y_low = *(s32 *)(obj + 0xB0);\n"
         "        if (y_low < 0) {\n            min_y = y_low;\n        } else {\n            max_y = y_low;\n        }\n"
         "        y = *(s32 *)(obj + 0x108);\n"
         "        if (max_y < y - a0_var || y + a0_var < min_y) return 0;\n",
         "        max_z = 0;\n        min_z = 0;\n        az = scr->unkA8.z;\n"
         "        if (az < 0) {\n            min_z = az;\n        } else {\n            max_z = az;\n        }\n"
         "        z = scr->unk100[0].z;\n"
         "        if (max_z < z - a0_var || z + a0_var < min_z) return 0;\n"),
    ])
    return sub1(s, "extern s32 func_8002EA24(u8 *obj, s32 *pos, s32 threshold, s32 r_sq);\n",
                "extern s32 func_8002EA24(Unk1F8002B8Rec *scr, LeafPos *pos, s32 threshold, s32 r_sq);\n")

V8 = "scr->unk00.v8002A458"
def fa458(s):
    R = [
        ("    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8;\n"),
        ("    s32 *p;\n", "    LeafPos *p;\n"),
        ("    *(u8 **)(scr + 0x60) = scr;\n    *(u8 **)(scr + 0x64) = scr + 0xC;\n"
         "    *(Vec3i *)(scr + 0xC8) = *(Vec3i *)(scr + 0xC);\n"
         "    dx = (*(s32 **)(scr + 0x64))[0] - (*(s32 **)(scr + 0x60))[0];\n"
         "    dy = (*(s32 **)(scr + 0x64))[1] - (*(s32 **)(scr + 0x60))[1];\n"
         "    dz = (*(s32 **)(scr + 0x64))[2] - (*(s32 **)(scr + 0x60))[2];\n",
         "    scr->unk60[0] = &%s.unk00[0];\n    scr->unk60[1] = &%s.unk00[1];\n"
         "    scr->unkC8 = %s.unk00[1];\n"
         "    dx = scr->unk60[1]->x - scr->unk60[0]->x;\n"
         "    dy = scr->unk60[1]->y - scr->unk60[0]->y;\n"
         "    dz = scr->unk60[1]->z - scr->unk60[0]->z;\n" % (V8, V8, V8)),
        ("    *(s16 *)(scr + 0xF8) = -ratan2(dy, hlen);\n    *(s16 *)(scr + 0xFA) = ratan2(dx, dz);\n"
         "    *(s16 *)(scr + 0xFC) = 0;\n",
         "    scr->unkF8.vx = -ratan2(dy, hlen);\n    scr->unkF8.vy = ratan2(dx, dz);\n"
         "    scr->unkF8.vz = 0;\n"),
        ("        func_80032854(id == 0, 0xB, (s32 *)(scr + 0xC8), (s16 *)(scr + 0xF8));\n",
         "        func_80032854(id == 0, 0xB, &scr->unkC8.x, &scr->unkF8.vx);\n"),
        ("    **(Vec3i **)(scr + 0x60) = **(Vec3i **)(scr + 0x64);\n"
         "    (*(s32 **)(scr + 0x64))[0] += qx * 4;\n"
         "    (*(s32 **)(scr + 0x64))[1] += qy * 4;\n"
         "    (*(s32 **)(scr + 0x64))[2] += qz * 4;\n",
         "    *scr->unk60[0] = *scr->unk60[1];\n"
         "    scr->unk60[1]->x += qx * 4;\n"
         "    scr->unk60[1]->y += qy * 4;\n"
         "    scr->unk60[1]->z += qz * 4;\n"),
        ("            s32 *pos;\n", "            LeafPos *pos;\n"),
        ("            pos = (s32 *)&SPAD->unkA8[id][i];\n", "            pos = &SPAD->unkA8[id][i];\n"),
        ("    *(s32 *)(scr + 0xA8) = (*(s32 **)(scr + 0x60))[0] - qx / 4;\n"
         "    *(s32 *)(scr + 0xAC) = (*(s32 **)(scr + 0x60))[1] - qy / 4;\n"
         "    *(s32 *)(scr + 0xB0) = (*(s32 **)(scr + 0x60))[2] - qz / 4;\n"
         "    if (func_80053614((s32 *)(scr + 0xA8), *(s32 **)(scr + 0x64), (s32 *)(scr + 0x100),\n"
         "                      (s16 *)(scr + 0xF8), (s32)work) != 0\n",
         "    scr->unkA8.x = scr->unk60[0]->x - qx / 4;\n"
         "    scr->unkA8.y = scr->unk60[0]->y - qy / 4;\n"
         "    scr->unkA8.z = scr->unk60[0]->z - qz / 4;\n"
         "    if (func_80053614(&scr->unkA8.x, &scr->unk60[1]->x, &scr->unk100[0].x,\n"
         "                      &scr->unkF8.vx, (s32)work) != 0\n"),
        ("            p = *(s32 **)(scr + 0x60);\n"
         "            dx = *(s32 *)(scr + 0x100) - p[0];\n"
         "            dy = *(s32 *)(scr + 0x104) - p[1];\n"
         "            dz = *(s32 *)(scr + 0x108) - p[2];\n",
         "            p = scr->unk60[0];\n"
         "            dx = scr->unk100[0].x - p->x;\n"
         "            dy = scr->unk100[0].y - p->y;\n"
         "            dz = scr->unk100[0].z - p->z;\n"),
        ("            dx = obj->unk_F4.x - p[0];\n            dy = obj->unk_F4.y - p[1];\n"
         "            dz = obj->unk_F4.z - p[2];\n",
         "            dx = obj->unk_F4.x - p->x;\n            dy = obj->unk_F4.y - p->y;\n"
         "            dz = obj->unk_F4.z - p->z;\n"),
        ("            func_80032854(id == 0, 0xA, (s32 *)(scr + 0x100), 0);\n",
         "            func_80032854(id == 0, 0xA, &scr->unk100[0].x, 0);\n"),
    ]
    s = edit(s, "func_8002A458", R)
    return sub1(s, "typedef struct { s32 x, y, z; } Vec3i;\n", "")

def f31b24(s):
    R = [
        ("    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8;\n"),
        ("    Vec3i32 *seg = (Vec3i32 *)scr;\n", "    Vec3i32 *seg = scr->unk00.v80031B24.unk00;\n"),
        ("        *(Vec3i32 **)(scr + 0x60) = &seg[0];\n        *(Vec3i32 **)(scr + 0x64) = &seg[1];\n",
         "        scr->unk60[0] = &seg[0];\n        scr->unk60[1] = &seg[1];\n"),
        ("            s32 *pos;\n", "            LeafPos *pos;\n"),
        ("            pos = (s32 *)&SPAD->unkA8[other][j];\n", "            pos = &SPAD->unkA8[other][j];\n"),
    ]
    return edit(s, "func_80031B24", R)

FS = ["func_8002EA24", "func_8002A458", "func_80031B24", "func_8002E838", "func_80031890"]
def build():
    src = rd("tmp/p2/lt/f02/v/17AFC.b2t2.c")
    g = game(rd("tmp/p2/lt/f02/game.b1c.h"))
    for f in (fea24, fa458, f31b24, fe838, f31890):
        src = f(src)
    return src, g

def wk(src, g):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)

def measure(src, g, fs=FS):
    wk(src, g)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/17AFC"] + fs, capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/17AFC"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    src, g = build()
    open("tmp/p2/lt/f02/v/17AFC.b3.c", "w", encoding="utf-8", newline=NL).write(src)
    open("tmp/p2/lt/f02/v/game.h.b3", "w", encoding="utf-8", newline=NL).write(g)
    if "measure" in sys.argv[1:]:
        measure(src, g)
    print("wrote b3")
