#!/usr/bin/env python3
# F02 batch 4 (prep): ScrPad.unk2B8 becomes Unk1F8002B8Union { rec; v80030D7C; v800321E8 }.
# func_80030D7C / func_800321E8 get their own layouts (func_8005344C's to / hit / normal and its
# Work_80053E9C work area at +0x38); Work_80053E9C (+ Cell_80052D00) moves from 3AB48.c to game.h;
# the typed bodies re-spell &SPAD->unk2B8 as &SPAD->unk2B8.rec.
# Base: v/17AFC.b3.c / v/game.h.b3 (batch 3 over the batch-2 t2 text), src/main/3AB48.c (HEAD).
# usage: f02b4.py [measure]
import os, re, shutil, subprocess, sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b2 as B
NL = chr(10)
edit, sub1, rd = B.edit, B.sub1, B.rd

def work_block(s3):
    i = s3.index("typedef struct {\n    s16 x;\n    s16 z;\n} Cell_80052D00;\n")
    j = s3.index("} Work_80053E9C;\n", i) + len("} Work_80053E9C;\n")
    return i, j

WORK_COMMENT = (
"/* A cell (x, z) of the 32x32 grid of 2000-unit cells that 3AB48's func_80052D00 walks\n"
" * (Work_80053E9C.unk88 / unk8C). */\n")
WORK_COMMENT2 = (
"/* The 0xEC-byte work area of 3AB48's stage-collision cast (func_80052D00 and its helpers, through\n"
" * D_800A33F4 and the W macro in 3AB48.c). func_8005344C / func_80053614 place it at their last\n"
" * argument; func_80053304 / func_80053584 at D_800EF9F8. 17AFC func_80030D7C / func_800321E8\n"
" * keep it in their layouts of the scratchpad area at 0x1F8002F0 (Unk1F8002B8Union). */\n")

V30 = """/* func_80030D7C's layout of the scratchpad area at 0x1F8002B8 (Unk1F8002B8Union.v80030D7C):
 * func_8005344C's to point, hit point and normal, and its work area, which runs over
 * Unk1F8002B8Rec's unk60..unk118. */
typedef struct {
    Vec3i32 unk00;                 /* to: the object's pos + vel; func_8005344C copies 16 bytes */
    s32 unk0C;                     /* read only as the 4th word of that copy */
    Vec3i32 unk10;                 /* the turn block's rotated velocity; then the hit point */
    s32 unk1C;                     /* no access */
    Vec3i32 unk20;                 /* the turn block's second rotated vector */
    s32 unk2C;                     /* no access */
    s16 unk30[3];                  /* the hit normal (func_8005344C writes [0..2]) */
    s16 unk36;                     /* no access */
    Work_80053E9C unk38;           /* func_8005344C's work area, +0x38..+0x123 */
} Unk1F8002B8_80030D7C;

/* func_800321E8's layout of the scratchpad area at 0x1F8002B8 (Unk1F8002B8Union.v800321E8):
 * func_8005344C's to point, hit point and normal, and its work area, which runs over
 * Unk1F8002B8Rec's unk60..unk118. */
typedef struct {
    Vec3i32 unk00;                 /* to: the D_80104E88 entry's +0x4 point plus its +0x1C step;
                                      func_8005344C copies 16 bytes */
    s32 unk0C;                     /* read only as the 4th word of that copy */
    Vec3i32 unk10;                 /* the hit point (func_8005344C writes it; not read here) */
    s32 unk1C;                     /* no access */
    s32 unk20[4];                  /* no access */
    s16 unk30[3];                  /* the hit normal (func_8005344C writes [0..2]; not read here) */
    s16 unk36;                     /* no access */
    Work_80053E9C unk38;           /* func_8005344C's work area, +0x38..+0x123 */
} Unk1F8002B8_800321E8;

/* ScrPad.unk2B8: the scratchpad's last 0x148 bytes, 0x1F8002B8..0x1F8003FF, used with more than
 * one layout. 17AFC's collision code uses rec (Unk1F8002B8Rec). func_80030D7C and func_800321E8
 * use their own (v80030D7C / v800321E8): they pass 0x1F8002F0 as func_8005344C's work-area
 * address, so its 0xEC-byte Work_80053E9C sits at +0x38..+0x123, over rec's unk60..unk118. Each
 * function uses exactly one member. */
typedef union {
    Unk1F8002B8Rec rec;
    Unk1F8002B8_80030D7C v80030D7C;
    Unk1F8002B8_800321E8 v800321E8;
} Unk1F8002B8Union;

"""

def game(g, workdef):
    # Work_80053E9C / Cell_80052D00 go ahead of the 0x1F8002B8 views.
    anchor = "/* Unk1F8002B8Rec.unk00 is per-function scratch:"
    cell, work = workdef
    g = sub1(g, anchor, WORK_COMMENT + cell + "\n" + WORK_COMMENT2 + work + "\n" + anchor)
    g = sub1(g, " * pointer; func_80030D7C / func_800321E8 lay out more than unk00 (func_8005344C's work area from\n"
                " * +0x38 through +0x123) and have no member here. */\n",
             " * pointer; func_80030D7C / func_800321E8 lay out more than unk00 (func_8005344C's work area from\n"
             " * +0x38 through +0x123), so their layouts are members of Unk1F8002B8Union instead. */\n")
    g = sub1(g, "/* 17AFC's view of the last 0x148 bytes of the scratchpad, 0x1F8002B8..0x1F8003FF (ScrPad.unk2B8).\n",
             "/* 17AFC's view of the last 0x148 bytes of the scratchpad, 0x1F8002B8..0x1F8003FF\n"
             " * (ScrPad.unk2B8.rec).\n")
    g = sub1(g, " * func_8002FDB0 / func_80030D7C / func_800321E8 address it directly. unk60 / unk6C hold point\n",
             " * func_8002FDB0 address it directly (func_80030D7C / func_800321E8 use their own layouts,\n"
             " * Unk1F8002B8Union). unk60 / unk6C hold point\n")
    g = sub1(g, " * func_80031B24 use their members; func_8002AB08 / func_80030D7C / func_800321E8 still use\n"
                " * it through a byte pointer. */\n",
             " * func_80031B24 use their members; func_8002AB08 still uses it through a byte pointer. */\n")
    # rewrap the record comment's tail (from the edited sentence to the end) at 100 columns
    a = g.index(" * func_8002FDB0 address it directly")
    b = g.index(" */\ntypedef struct {\n    Unk1F8002B8Unk00 unk00;")
    words = " ".join(l[3:] for l in g[a:b].split("\n")).split()
    lines, cur = [], " *"
    for w in words:
        if len(cur) + 1 + len(w) > 100:
            lines.append(cur); cur = " *"
        cur += " " + w
    lines.append(cur)
    g = g[:a] + "\n".join(lines) + g[b:]
    g = sub1(g, "} Unk1F8002B8Rec;\n\n", "} Unk1F8002B8Rec;\n\n" + V30)
    g = sub1(g, " * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48.  unk2B8: the record at\n"
                " * 0x1F8002B8 (Unk1F8002B8Rec), to the end of the scratchpad; the scratchpad is shared scratch\n"
                " * that other code also uses with its own views (see Unk1F8002B8Rec). */\n",
             " * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48.  unk2B8: the area at\n"
             " * 0x1F8002B8 (Unk1F8002B8Union), to the end of the scratchpad; the scratchpad is shared scratch\n"
             " * that other code also uses with its own views (see Unk1F8002B8Rec). */\n")
    g = sub1(g, "    Unk1F8002B8Rec unk2B8;\n} ScrPad;", "    Unk1F8002B8Union unk2B8;\n} ScrPad;")
    return g

def respell(s):
    for fn in ["func_8002A458", "func_8002CA8C", "func_8002EBDC", "func_80031B24"]:
        s = edit(s, fn, [("    Unk1F8002B8Rec *scr = &SPAD->unk2B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;\n")])
    s = edit(s, "func_8002F2D0", [("    m = &SPAD->unk2B8.unkD8;\n", "    m = &SPAD->unk2B8.rec.unkD8;\n"),
                                  ("    scr = &SPAD->unk2B8;\n", "    scr = &SPAD->unk2B8.rec;\n")])
    s = edit(s, "func_8002F770", [("    scr = &SPAD->unk2B8;\n", "    scr = &SPAD->unk2B8.rec;\n")])
    return s

def f30d7c(s):
    R = [
        ("    u8 *scr;\n", "    Unk1F8002B8_80030D7C *scr;\n"),
        ("    scr = (u8 *)0x1F8002B8;\n", "    scr = &SPAD->unk2B8.v80030D7C;\n"),
        ("            *(s32 *)(scr + 0x10) = (Judge", "            scr->unk10.x = (Judge"),
        ("            *(s32 *)(scr + 0x14) = obj->vel.y;\n", "            scr->unk10.y = obj->vel.y;\n"),
        ("            *(s32 *)(scr + 0x18) = (Judge", "            scr->unk10.z = (Judge"),
        ("            *(s32 *)(scr + 0x20) = *(s32 *)(scr + 0x10);\n", "            scr->unk20.x = scr->unk10.x;\n"),
        ("            *(s32 *)(scr + 0x24) = (Judge[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x14)\n"
         "                                    - Judge[half & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;\n"
         "            *(s32 *)(scr + 0x28) = (Judge[half & 0xFFF] * *(s32 *)(scr + 0x14)\n"
         "                                    + Judge[(half + 0x400) & 0xFFF] * *(s32 *)(scr + 0x18)) >> 12;\n"
         "            obj->vel.x = (Judge[(work - temp + 0x400) & 0xFFF] * *(s32 *)(scr + 0x20)\n"
         "                                    - Judge[(work - temp) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;\n"
         "            obj->vel.y = *(s32 *)(scr + 0x24);\n"
         "            obj->vel.z = (Judge[(work - temp) & 0xFFF] * *(s32 *)(scr + 0x20)\n"
         "                                    + Judge[(work - temp + 0x400) & 0xFFF] * *(s32 *)(scr + 0x28)) >> 12;\n",
         "            scr->unk20.y = (Judge[(half + 0x400) & 0xFFF] * scr->unk10.y\n"
         "                                    - Judge[half & 0xFFF] * scr->unk10.z) >> 12;\n"
         "            scr->unk20.z = (Judge[half & 0xFFF] * scr->unk10.y\n"
         "                                    + Judge[(half + 0x400) & 0xFFF] * scr->unk10.z) >> 12;\n"
         "            obj->vel.x = (Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.x\n"
         "                                    - Judge[(work - temp) & 0xFFF] * scr->unk20.z) >> 12;\n"
         "            obj->vel.y = scr->unk20.y;\n"
         "            obj->vel.z = (Judge[(work - temp) & 0xFFF] * scr->unk20.x\n"
         "                                    + Judge[(work - temp + 0x400) & 0xFFF] * scr->unk20.z) >> 12;\n"),
        ("        *(s32 *)(scr + 0x0) = obj->pos.x + obj->vel.x;\n"
         "        *(s32 *)(scr + 0x4) = obj->pos.y + obj->vel.y;\n"
         "        *(s32 *)(scr + 0x8) = obj->pos.z + obj->vel.z;\n",
         "        scr->unk00.x = obj->pos.x + obj->vel.x;\n"
         "        scr->unk00.y = obj->pos.y + obj->vel.y;\n"
         "        scr->unk00.z = obj->pos.z + obj->vel.z;\n"),
        ("        nrm = (s16 *)(scr + 0x30);\n"
         "        temp = func_8005344C((s32 *)&obj->pos, (s32 *)scr, (s32 *)(scr + 0x10), nrm, (s32)(scr + 0x38));\n",
         "        nrm = scr->unk30;\n"
         "        temp = func_8005344C(&obj->pos.x, &scr->unk00.x, &scr->unk10.x, nrm, (s32)&scr->unk38);\n"),
        ("                func_80032854(obj->owner, 0xE, (s32 *)(scr + 0x10), nrm);\n",
         "                func_80032854(obj->owner, 0xE, &scr->unk10.x, nrm);\n"),
        ("            obj->pos = *(Vec3i32 *)(scr + 0x10);\n", "            obj->pos = scr->unk10;\n"),
        ("*(s16 *)(scr + 0x32) >= -0x7FF", "scr->unk30[1] >= -0x7FF", 2),
        ("            obj->pos = *(Vec3i32 *)scr;\n", "            obj->pos = scr->unk00;\n"),
    ]
    s = edit(s, "func_80030D7C", R)
    return sub1(s, " * returned normal, func_80032854 cues, and the rest / re-hop logic. Scratch\n"
                   " * vectors live in the scratchpad record at 0x1F8002B8. */\n",
                " * returned normal, func_80032854 cues, and the rest / re-hop logic. Scratch\n"
                " * vectors live in its layout of the scratchpad area at 0x1F8002B8\n"
                " * (Unk1F8002B8_80030D7C). */\n")

def f321e8(s):
    R = [
        ("    s32 *sp = (s32 *)0x1F8002B8;\n", "    Unk1F8002B8_800321E8 *scr = &SPAD->unk2B8.v800321E8;\n"),
        ("            *(Vec3_copy *)(base + 0x10) = *(Vec3_copy *)(base + 4);\n",
         "            *(Vec3i32 *)(base + 0x10) = *(Vec3i32 *)(base + 4);\n"),
        ("            sp[0] = *(s32 *)(base + 4)", "            scr->unk00.x = *(s32 *)(base + 4)"),
        ("            sp[1] = *(s32 *)(base + 8)", "            scr->unk00.y = *(s32 *)(base + 8)"),
        ("                s32 arg5 = (s32)sp + 0x38;\n", "                s32 arg5 = (s32)&scr->unk38;\n"),
        ("                sp[2] = *(s32 *)(base + 0xC)", "                scr->unk00.z = *(s32 *)(base + 0xC)"),
        ("the loop-invariant sp+0x38 into", "the loop-invariant &scr->unk38 into"),
        ("func_8005344C((s32 *)(base + 4), sp, (s32 *)((u8 *)sp + 0x10), (s16 *)((u8 *)sp + 0x30), arg5)",
         "func_8005344C((s32 *)(base + 4), &scr->unk00.x, &scr->unk10.x, scr->unk30, arg5)"),
        ("                    *(Vec3_copy *)(base + 4) = *(Vec3_copy *)sp;\n",
         "                    *(Vec3i32 *)(base + 4) = scr->unk00;\n"),
    ]
    return edit(s, "func_800321E8", R)

def f3ab48(s3):
    i, j = work_block(s3)
    blk = s3[i:j]
    k = blk.index("typedef struct {\n    s32 unk0;")
    cell, work = blk[:k].rstrip("\n") + "\n", blk[k:]
    s3 = s3[:i] + s3[j:]
    s3 = sub1(s3, "extern s32 func_80053694(s32 *, s16 *);\n\n\n#define W",
              "extern s32 func_80053694(s32 *, s16 *);\n\n#define W")
    return s3, (cell, work)

def build():
    src = rd("tmp/p2/lt/f02/v/17AFC.b3.c")
    g = rd("tmp/p2/lt/f02/v/game.h.b3")
    s3 = rd("tmp/p2/lt/f02/3AB48.head.c")
    s3, wd = f3ab48(s3)
    g = game(g, wd)
    src = respell(src); src = f30d7c(src); src = f321e8(src)
    return src, g, s3

FS = ["func_80030D7C", "func_800321E8", "func_8002A458", "func_8002CA8C", "func_8002EBDC",
      "func_8002F2D0", "func_8002F770", "func_80031B24"]

def wk(src, g, s3):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)
    open("tmp/p2/wk/src/main/3AB48.c", "w", encoding="utf-8", newline=NL).write(s3)

def measure(src, g, s3, fs=FS):
    wk(src, g, s3)
    for tu, f in (("main/17AFC", fs), ("main/3AB48", [])):
        r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", tu] + f, capture_output=True, text=True)
        print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/17AFC", "main/3AB48"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    src, g, s3 = build()
    for p, t in (("v/17AFC.b4.c", src), ("v/game.h.b4", g), ("v/3AB48.b4.c", s3)):
        open("tmp/p2/lt/f02/" + p, "w", encoding="utf-8", newline=NL).write(t)
    if "measure" in sys.argv[1:]:
        measure(src, g, s3)
    print("wrote b4")
