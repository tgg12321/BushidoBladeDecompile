#!/usr/bin/env python3
# FZZ 2B344 + 3AB48 groups (on the F04 batch, ../f04/f5c4.py).
# usage: f2b.py [opt=<name>,...]   writes tmp/p2/f2b/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f04"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import f5c4 as P
sys.argv = _argv
OUT = "tmp/p2/f2b/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = P.sub1
fn = P.fn

# ---------------------------------------------------------------- 2B344: SceneRec
def scene(w):
    # The record func_8003FA24 fills: +0 the s16 func_80017D84 returns (read back with lhu),
    # +4..+0x13 the 16 bytes func_8003F824 copies into Scene.quads (its fields per func_8003FA24's stores).
    w = sub1(w, """typedef struct {
    /* 0x00 */ s32 v[4];
} SceneQuad;

typedef struct {
    /* 0x00 */ s32 unk0;
    /* 0x04 */ SceneQuad quad;
""", """/* The 16 bytes func_8003FA24 fills at SceneRec +0x04 and func_8003F824 copies into Scene.quads:
   the record's id (unk0), two zero bytes, the object's matrix (its +0x18), its point table end and
   the address of the record's inner.objs[3]. */
typedef struct {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 *unk4;
    /* 0x08 */ u8 *unk8;
    /* 0x0C */ void *unkC;
} SceneQuad;

typedef struct {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u8 unk2[2];
    /* 0x04 */ SceneQuad quad;
""")
    def g(b):
        b = sub1(b, "    *(s16 *)rec = func_80017D84((u8 *)&init);\n", "    rec->unk0 = func_80017D84((u8 *)&init);\n")
        b = sub1(b, """    *(u16 *)((u8 *)rec + 4) = *(u16 *)rec;
    *(u8 **)((u8 *)rec + 8) = obj + 0x18;
    *(u8 **)((u8 *)rec + 0xC) = *(u8 **)(obj + 0x60);
    *(void **)((u8 *)rec + 0x10) = (u8 *)rec + 0x2C;
    *((u8 *)rec + 6) = 0;
    *((u8 *)rec + 7) = 0;
""", """    rec->quad.unk0 = rec->unk0;
    rec->quad.unk4 = obj + 0x18;
    rec->quad.unk8 = *(u8 **)(obj + 0x60);
    rec->quad.unkC = &rec->inner.objs[3];
    rec->quad.unk2 = 0;
    rec->quad.unk3 = 0;
""")
        if "srcpun" not in OPT:
            b = sub1(b, "for (n = *(s16 *)src++, count = n; n != 0; n = *(s16 *)src++, count = n) {",
                     "for (n = (s16)*src++, count = n; n != 0; n = (s16)*src++, count = n) {")
        return b
    return fn(w, "func_8003FA24", g)

def rect3DE14(w):
    # func_8003DE14's rect is a RECT (its callers build one; it goes to StoreImage / LoadImage)
    def g(b):
        b = sub1(b, "void func_8003DE14(s16 *rect, s32 count) {", "void func_8003DE14(RECT *rect, s32 count) {")
        for a, c in (("((u16 *)rect)[0]", "rect->x"), ("((u16 *)rect)[1]", "rect->y"), ("((u16 *)rect)[2]", "rect->w"),
                     ("((u16 *)rect)[3]", "rect->h"), ("rect[1]", "rect->y"), ("rect[2]", "rect->w"), ("rect[3]", "rect->h")):
            b = b.replace(a, c)
        return b
    w = fn(w, "func_8003DE14", g)
    for f in ("func_8003E0E0", "func_8003E120"):
        def h(b):
            b = sub1(b, "    s16 buf[4];\n", "    RECT buf;\n")
            for k, m in (("0", "x"), ("1", "y"), ("2", "w"), ("3", "h")):
                b = sub1(b, "buf[%s] = " % k, "buf.%s = " % m)
            return sub1(b, "func_8003DE14(buf, ", "func_8003DE14(&buf, ")
        w = fn(w, f, h)
    return w

REC3D40 = """/* 0x800A3D40: 24-byte records func_8003D774 starts and func_8003D7B4 advances: the bit stream
   bitstream_ReadBits reads (unk0, its u32 words) and six s16 values func_8003D7B4 adds the decoded
   deltas to and returns (3AB48 func_8005490C reads them as an offset and a rotation). */
typedef struct {
    u32 unk0[3];
    s16 unkC[6];
} Unk800A3D40Rec;

"""

def rec3D40(w, h, g_):
    h = sub1(h, "extern s32 D_800A3D40;\n", "extern Unk800A3D40Rec D_800A3D40[];\n")
    anchor = "/* 5ED34's work block: the 0x14 bytes func_8006E534 keeps"
    g_ = sub1(g_, anchor, REC3D40 + anchor)
    def a(b):
        b = sub1(b, "    s32 *ptr = (s32 *)((u8 *)&D_800A3D40 + arg1 * 24);\n", "    Unk800A3D40Rec *ptr = &D_800A3D40[arg1];\n")
        b = sub1(b, "    ptr[0] = arg0;\n    ptr[1] = 0;\n    ptr[2] = 0;\n", "    ptr->unk0[0] = arg0;\n    ptr->unk0[1] = 0;\n    ptr->unk0[2] = 0;\n")
        for k, o in enumerate(("0x16", "0x14", "0x12", "0x10", "0xE", "0xC")):
            b = sub1(b, "*(s16 *)((u8 *)ptr + %s) = 0;" % o, "ptr->unkC[%d] = 0;" % (5 - k))
        return b
    w = fn(w, "func_8003D774", a)
    def c(b):
        b = sub1(b, "    u8 *base = (u8 *)&D_800A3D40 + (arg0 * 24);\n    u8 *p;\n", "    Unk800A3D40Rec *base = &D_800A3D40[arg0];\n")
        b = sub1(b, "bitstream_ReadBits((s32 *)base, ", "bitstream_ReadBits(base->unk0, ", 2)
        b = sub1(b, "        p = base + i * 2;\n        *(u16 *)(p + 0xC) = (u16)(*(u16 *)(p + 0xC) + val);\n",
                 "        base->unkC[i] += val;\n")
        b = sub1(b, "    return (s16 *)(base + 0xC);\n", "    return base->unkC;\n")
        return b
    w = fn(w, "func_8003D7B4", c)
    return w, h, g_

def script(w, h):
    # D_800A3844 is the stage script's byte cursor (func_8003B5A4 reads a command byte and its
    # operands from it); D_800900EC the scripts' start addresses; D_800A3878 a 4-byte stage record in
    # one of the tables the script points D_800A3894 / D_800A385C at, or in the script itself.
    h = sub1(h, "extern s32 D_800A3844;\n", "extern u8 *D_800A3844;\n")
    h = sub1(h, "extern s32 D_800A3878;\n", "extern u8 *D_800A3878;\n")
    h = sub1(h, "extern u8 D_800900EC;\n", "extern u8 *D_800900EC[];\n")
    w = sub1(w, "    D_800A3844 = ((s32 *)&D_800900EC)[arg0];\n", "    D_800A3844 = D_800900EC[arg0];\n")
    w = sub1(w, "        s32 ptr = D_800A3844;\n", "        u8 *ptr = D_800A3844;\n")
    w = sub1(w, "        cmd = *(u8 *)ptr;\n", "        cmd = *ptr;\n")
    w = w.replace("func_8003B3A4((u8 *)D_800A3844)", "func_8003B3A4(D_800A3844)")
    w = w.replace("func_8003B484((u8 *)D_800A3844)", "func_8003B484(D_800A3844)")
    w = w.replace("u8 *p = (u8 *)D_800A3844;", "u8 *p = D_800A3844;")
    w = w.replace("D_800A3844 = (s32)(p + 1);", "D_800A3844 = p + 1;").replace("D_800A3844 = (s32)(p + 2);", "D_800A3844 = p + 2;")
    w = w.replace("D_800A3894 = (u8 *)D_800A3844;", "D_800A3894 = D_800A3844;").replace("D_800A385C = (u8 *)D_800A3844;", "D_800A385C = D_800A3844;")
    w = w.replace("((u8 *)D_800A3878)[", "D_800A3878[")
    w = w.replace("u8 *q = (u8 *)D_800A3878;", "u8 *q = D_800A3878;").replace("p = (u8 *)D_800A3878;", "p = D_800A3878;")
    left = [l.strip() for l in w.split(NL) if re.search(r"D_800A3844|D_800A3878|D_800900EC", l) and re.search(r"\((s32|u8 \*|s32 \*)\)", l)]
    for l in left:
        print("LEFT", l)
    return w, h

def fixes(out):
    # rev-f04 round 1: func_8006F528's alias labelled; header comments; redundant locals removed
    # (simplest form); measured scores added to carried FAKE labels; func_8007855C's out.
    e, t, s, a, w, g = (out[k] for k in ("5ED34.c", "64FD8.c", "51268.c", "3AB48.c", "2B344.c", "game.h"))
    def f528(b):
        b = sub1(b, "        Unk800A35C4Rec *offset;\n",
                 "        /* FAKE: one-use alias of D_800A35C4 for the offset store: written\n"
                 "           `D_800A35C4->unk_10[0] = offset_x`, the D_800A35C4 load (`lw a1`) sinks from the\n"
                 "           state test to the store, the test's 2 moves from $a2 to $a1 and offset_x from\n"
                 "           $v0 to $v1 (score 8). */\n"
                 "        Unk800A35C4Rec *blk;\n")
        b = sub1(b, "        offset = D_800A35C4;\n", "        blk = D_800A35C4;\n")
        b = sub1(b, "        offset->unk_10[0] = offset_x;\n", "        blk->unk_10[0] = offset_x;\n")
        return b
    e = fn(e, "func_8006F528", f528)
    g = sub1(g, "(func_80052D00 and its helpers, through\n * D_800A33F4 and the W macro in 3AB48.c).",
             "(func_80052D00 and its helpers, through\n * D_800A33F4).")
    g = sub1(g, "the initial targets of D_800A34E4 (u8 *) / D_800A34E8 (u32 *)", "the initial targets of D_800A34E4 (u32 *) / D_800A34E8 (u32 *)")
    g = sub1(g, " * - unk_7C: the bytes func_80070F78 uploads per player; unk_80: the bytes func_800720FC reads.",
             " * - unk_7C: per player a row of VRAM RECTs (64 bytes) func_80070F78 hands LoadImage as the\n"
             " *   destination; unk_80: the bytes func_800720FC reads.")
    def d88(b):
        b = sub1(b, "            u32 *p_a;\n", "")
        b = sub1(b, "                p_a = (u32 *)(g_gpu_ot_ptr + (s32)(entry * 4));\n                D_800A34E4 = p_a;\n",
                 "                D_800A34E4 = (u32 *)(g_gpu_ot_ptr + (s32)(entry * 4));\n")
        b = sub1(b, "(*p_a & 0xFFFFFF)", "(*D_800A34E4 & 0xFFFFFF)")
        b = sub1(b, """                {
                    u32 *p_a2 = D_800A34E4;
                    *p_a2 = ((u32)D_800A34E8 & 0xFFFFFF) | (*p_a2 & 0xFF000000);
                }
""", """                *D_800A34E4 = ((u32)D_800A34E8 & 0xFFFFFF) | (*D_800A34E4 & 0xFF000000);
""")
        return b
    s = fn(s, "func_80068D88", d88)
    for f in ("func_80053304", "func_8005344C"):
        def pp(b):
            b = sub1(b, "    Work_80053E9C *p;\n", "")
            b = sub1(b, "        p = D_800A33F4;\n", "")
            return re.sub(r"\bp->", "D_800A33F4->", b)
        a = fn(a, f, pp)
    def e950(b):
        b = sub1(b, "    Unk8006E950Head *s1 = a1;\n", "")
        return re.sub(r"\bs1\b", "a1", b)
    e = fn(e, "func_8006E950", e950)
    def b9d0(b):
        b = sub1(b, "    u8 *p;\n", "")
        b = sub1(b, "        u8 *q = D_800A3878;\n        u8 qf = q[3];\n", "        u8 qf = D_800A3878[3];\n")
        b = sub1(b, "if (q[3] & 0x20)", "if (D_800A3878[3] & 0x20)")
        b = sub1(b, "    p = D_800A3878;\n    flags = p[3];\n", "    flags = D_800A3878[3];\n")
        b = sub1(b, "func_80054884(D_800A376C, p[0], ", "func_80054884(D_800A376C, D_800A3878[0], ")
        return b
    w = fn(w, "func_8003B9D0", b9d0)
    e = sub1(e, "                               * Same construct at the confirm and locked sites. */",
             "                               * Same construct at the confirm and locked sites.\n"
             "                               * Written inline: score 27. */")
    e = sub1(e, "                     * `li a1,0x60` at each call site. */", "                     * `li a1,0x60` at each call site (score 7). */")
    e = sub1(e, "             * from hoisting it into a callee-save. */\n            s32 cx = 0x140;",
             "             * from hoisting it into a callee-save. Both literals inline (cx and cy):\n"
             "             * score 44. */\n            s32 cx = 0x140;")
    e = sub1(e, "         * for i == 0 at .L8006EF14).  Family: duplicated-statement-into-arms. */",
             "         * for i == 0 at .L8006EF14).  Family: duplicated-statement-into-arms.\n"
             "         * One shared copy: score 20. */")
    t = sub1(t, "       Same shape as func_80078654's `zero` and func_80070C70's `c60`. */\n    s32 abr;",
             "       Same shape as func_80078654's `zero` and func_80070C70's `c60`.\n"
             "       The literal at every call: score 12. */\n    s32 abr;")
    def c55(b):
        b = sub1(b, "        s16 *p_struct = D_800A35F8->out34;\n", "        s16 *out = D_800A35F8->out34;\n")
        return sub1(b, "new_val < p_struct[5];", "new_val < out[5];")
    t = fn(t, "func_8007855C", c55)
    out.update({"5ED34.c": e, "64FD8.c": t, "51268.c": s, "3AB48.c": a, "2B344.c": w, "game.h": g})
    return out

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(P.write())
    out["2B344.c"] = scene(out["2B344.c"])
    out["2B344.c"], out["bb2.h"], out["game.h"] = rec3D40(out["2B344.c"], out["bb2.h"], out["game.h"])
    out["2B344.c"], out["bb2.h"] = script(out["2B344.c"], out["bb2.h"])
    out = fixes(out)
    if "rect" in OPT:
        out["2B344.c"] = rect3DE14(out["2B344.c"])
        out["bb2.h"] = sub1(out["bb2.h"], "extern void func_8003DE14(s16 *, s32);", "extern void func_8003DE14(RECT *, s32);")
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f2b", sorted(OPT))
