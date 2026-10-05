#!/usr/bin/env python3
# F02 batch 4 v2 (on HEAD 3351da420, batch 3 landed): ScrPad.unk2B8 becomes Unk1F8002B8Union
# { rec; v8005344C }. func_80030D7C and func_800321E8 lay the bytes out the same way (func_8005344C's
# to / hit / normal and its Work_80053E9C work area at +0x38), so they share ONE struct,
# Unk1F8002B8_8005344C (the batch-3 review's lesson: no per-function structs for one layout).
# Work_80053E9C (+ Cell_80052D00) moves from 3AB48.c to game.h; the rec users re-spell
# &SPAD->unk2B8 as &SPAD->unk2B8.rec. Body edits reuse f02b4.py's (renamed).
# usage: f02b4v2.py [measure]
import sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b4 as P
sub1, rd, NL = P.sub1, P.rd, P.NL
P.WORK_COMMENT2 = """/* The 0xEC-byte work area of 3AB48's stage-collision cast (func_80052D00 and its helpers, through
 * D_800A33F4 and the W macro in 3AB48.c). func_8005344C / func_80053614 place it at their last
 * argument; func_80053304 / func_80053584 at D_800EF9F8. 17AFC func_80030D7C / func_800321E8 pass
 * 0x1F8002F0, its place in their scratchpad layout (Unk1F8002B8_8005344C.unk38). */
"""

V = """/* The layout func_80030D7C and func_800321E8 give the scratchpad area at 0x1F8002B8
 * (Unk1F8002B8Union.v8005344C): the to point, hit point and normal they pass func_8005344C, and
 * its work area, which runs over Unk1F8002B8Rec's unk60..unk118. */
typedef struct {
    Vec3i32 unk00;                 /* to; func_8005344C copies 16 bytes from it */
    s32 unk0C;                     /* read only as the 4th word of that copy */
    Vec3i32 unk10;                 /* the hit point; func_80030D7C's turn block first keeps a
                                      rotated velocity here */
    s32 unk1C;                     /* no access */
    Vec3i32 unk20;                 /* func_80030D7C's turn block's second rotated vector
                                      (func_800321E8 does not access it) */
    s32 unk2C;                     /* no access */
    s16 unk30[3];                  /* the hit normal */
    s16 unk36;                     /* no access */
    Work_80053E9C unk38;           /* func_8005344C's work area, +0x38..+0x123 */
} Unk1F8002B8_8005344C;

/* ScrPad.unk2B8: the scratchpad's last 0x148 bytes, 0x1F8002B8..0x1F8003FF, which 17AFC uses with
 * two layouts. Its collision code uses rec (Unk1F8002B8Rec). func_80030D7C and func_800321E8 use
 * v8005344C: they pass 0x1F8002F0 as func_8005344C's work-area address, so its 0xEC-byte
 * Work_80053E9C sits at +0x38..+0x123, over rec's unk60..unk118. */
typedef union {
    Unk1F8002B8Rec rec;
    Unk1F8002B8_8005344C v8005344C;
} Unk1F8002B8Union;

"""

def rewrap(g, a_mark, b_mark):
    a = g.index(a_mark)
    b = g.index(b_mark)
    words = " ".join(l[3:] for l in g[a:b].split("\n")).split()
    lines, cur = [], " *"
    for w in words:
        if len(cur) + 1 + len(w) > 100:
            lines.append(cur); cur = " *"
        cur += " " + w
    lines.append(cur)
    return g[:a] + "\n".join(lines) + g[b:]

def game(g, workdef):
    anchor = "/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch."
    cell, work = workdef
    g = sub1(g, anchor, P.WORK_COMMENT + cell + "\n" + P.WORK_COMMENT2 + work + "\n" + anchor)
    g = sub1(g, " * pointer. func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument\n"
                " * block, its work area from +0x38 through +0x123) and have no member here. raw sizes the union\n"
                " * to 0x60. */\n",
             " * pointer. func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument\n"
             " * block, its work area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of\n"
             " * Unk1F8002B8Union. raw sizes the union to 0x60. */\n")
    g = sub1(g, "/* 17AFC's view of the last 0x148 bytes of the scratchpad, 0x1F8002B8..0x1F8003FF (ScrPad.unk2B8).\n",
             "/* 17AFC's view of the last 0x148 bytes of the scratchpad, 0x1F8002B8..0x1F8003FF\n"
             " * (ScrPad.unk2B8.rec).\n")
    g = sub1(g, " * func_8002FDB0 / func_80030D7C / func_800321E8 address it directly. unk60 / unk6C hold point\n",
             " * func_8002FDB0 address it directly (func_80030D7C / func_800321E8 use Unk1F8002B8_8005344C,\n"
             " * Unk1F8002B8Union). unk60 / unk6C hold point\n")
    g = sub1(g, " * use its two points; func_8002AB08 / func_80030D7C (u8 *) and func_800321E8 (s32 *) still\n"
                " * use it through raw pointers. */\n",
             " * use its two points; func_8002AB08 still uses it through its u8 * pointer. */\n")
    g = rewrap(g, " * func_8002FDB0 address it directly", " */\ntypedef struct {\n    Unk1F8002B8Unk00 unk00;")
    g = sub1(g, "} Unk1F8002B8Rec;\n\n", "} Unk1F8002B8Rec;\n\n" + V)
    g = sub1(g, " * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48.  unk2B8: the record at\n"
                " * 0x1F8002B8 (Unk1F8002B8Rec), to the end of the scratchpad; the scratchpad is shared scratch\n"
                " * that other code also uses with its own views (see Unk1F8002B8Rec). */\n",
             " * func_80023F08 hands func_800207C8 its character's unkA8 / unk00 / unk48.  unk2B8: the area at\n"
             " * 0x1F8002B8 (Unk1F8002B8Union), to the end of the scratchpad; the scratchpad is shared scratch\n"
             " * that other code also uses with its own views (see Unk1F8002B8Rec). */\n")
    g = sub1(g, "    Unk1F8002B8Rec unk2B8;\n} ScrPad;", "    Unk1F8002B8Union unk2B8;\n} ScrPad;")
    return g

def rename(s):
    for a, b in (("Unk1F8002B8_80030D7C", "Unk1F8002B8_8005344C"), ("Unk1F8002B8_800321E8", "Unk1F8002B8_8005344C"),
                 ("v80030D7C", "v8005344C"), ("v800321E8", "v8005344C")):
        s = s.replace(a, b)
    return s

def build():
    src = rd("src/main/17AFC.c")
    g = rd("include/game.h")
    s3 = rd("src/main/3AB48.c")
    assert "8005344C *" not in src and "v8005344C" not in g
    s3, wd = P.f3ab48(s3)
    g = game(g, wd)
    src = P.respell(src); src = P.f30d7c(src); src = P.f321e8(src)
    src = rename(src)
    return src, g, s3

if __name__ == "__main__":
    src, g, s3 = build()
    for p, t in (("v/17AFC.b4v2.c", src), ("v/game.h.b4v2", g), ("v/3AB48.b4v2.c", s3)):
        open("tmp/p2/lt/f02/" + p, "w", encoding="utf-8", newline=NL).write(t)
    if "measure" in sys.argv[1:]:
        P.measure(src, g, s3)
    print("wrote b4v2")
