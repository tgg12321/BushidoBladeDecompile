#!/usr/bin/env python3
# LoadImage / StoreImage / ClearImage family: libgpu.h gains PsyQ's prototypes (int return, as the callers'
# register allocation shows: 368E4 func_80048864 moves with a void declaration), sys.c's definitions return
# addque2's result as PsyQ does, every local declaration goes and every caller passes a RECT * and a u32 *.
# The VRAM rectangles become RECT (data, locals, the local RECT twins Rect / Rect77D94 / Rect_8006ECF4 /
# SLocal). Base: 370450dcc (fclose1 landed).
# usage: fimg1.py   writes tmp/p2/fimg1/ (scratch only); BASE_REV env (default 370450dcc)
import os, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "fclose"))
OUT = "tmp/p2/fimg1/"
BASE_REV = os.environ.get("BASE_REV", "370450dcc")
CHAIN = os.environ.get("CHAIN", "0") == "1"   # apply fclose1 first (scratch, before it landed)
FILES = {"2B344.c": "src/main/2B344.c", "3AB48.c": "src/main/3AB48.c", "51268.c": "src/main/51268.c",
         "5ED34.c": "src/main/5ED34.c", "64FD8.c": "src/main/64FD8.c", "6CF8.c": "src/main/6CF8.c",
         "31D3C.c": "src/main/31D3C.c", "368E4.c": "src/main/368E4.c", "ext.c": "src/main/psxsdk/libgpu/ext.c",
         "sys.c": "src/main/psxsdk/libgpu/sys.c", "libgpu.h": "include/psxsdk/libgpu.h",
         "game.h": "include/game.h", "bb2.h": "include/bb2.h"}

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, a[:70]
        t = t.replace(a, b)
    return t

def span(t, header):
    i = t.index(header)
    return i, t.index(NL + "}" + NL, i)

def in_fn(t, header, fx):
    i, j = span(t, header)
    return t[:i] + fx(t[i:j]) + t[j:]

def members(b, name, sep):
    for k, m in enumerate("xywh"):
        b = b.replace("%s[%d]" % (name, k), "%s%s%s" % (name, sep, m))
    return b

# ---- library side
def libgpu(g):
    return rep(g, [("extern s32 MoveImage(RECT *, s32, s32);\n",
                    "extern s32 ClearImage(RECT *, u8, u8, u8);\n"
                    "extern s32 LoadImage(RECT *, u32 *);  /* PsyQ: u_long *p */\n"
                    "extern s32 StoreImage(RECT *, u32 *); /* PsyQ: u_long *p */\n"
                    "extern s32 MoveImage(RECT *, s32, s32);\n")])

def sysc(t):
    return rep(t, [
        ("void ClearImage(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {\n    checkRECT(g_str_clearimage, arg0);\n    g_gpu",
         "s32 ClearImage(RECT *arg0, u8 arg1, u8 arg2, u8 arg3) {\n    checkRECT(g_str_clearimage, arg0);\n    return g_gpu"),
        ("void LoadImage(RECT *a0, u32 *a1) {\n    checkRECT(g_str_loadimage, a0);\n    g_gpu",
         "s32 LoadImage(RECT *a0, u32 *a1) {\n    checkRECT(g_str_loadimage, a0);\n    return g_gpu"),
        ("void StoreImage(RECT *a0, u32 *a1) {\n    checkRECT(g_str_storeimage, a0);\n    g_gpu",
         "s32 StoreImage(RECT *a0, u32 *a1) {\n    checkRECT(g_str_storeimage, a0);\n    return g_gpu"),
    ])

def ext(t):
    t = rep(t, [
        ("u16 LoadTPage(s32 a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {",
         "u16 LoadTPage(u32 *a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {"),
        ("u16 LoadClut(s32 a0, s32 a1, s32 a2) {", "u16 LoadClut(u32 *a0, s32 a1, s32 a2) {"),
        ("u16 LoadClut2(s32 a0, s32 a1, s32 a2) {", "u16 LoadClut2(u32 *a0, s32 a1, s32 a2) {"),
    ])
    assert t.count("    s16 buf[4];\n") == 3 and t.count("    LoadImage((s32)buf, a0);\n") == 3
    t = t.replace("    s16 buf[4];\n", "    RECT buf;\n").replace("    LoadImage((s32)buf, a0);\n", "    LoadImage(&buf, a0);\n")
    return members(t, "buf", ".")

# ---- game side
def s2B344(t):
    t = rep(t, [("extern void LoadImage(s32, s32);\n", ""), ("extern void StoreImage(s32 *, u16 *);\n", ""),
                ("    LoadImage((s32)&D_800A3220, (s32)&D_80090178);\n", "    LoadImage(&D_800A3220, &D_80090178);\n"),
                (" * inner bound's `+ rect[2] - rect[2]` detour", " * inner bound's `+ rect->w - rect->w` detour")])
    def de14(b):
        b = rep(b, [
            ("void func_8003DE14(s16 *rect, s32 count) {", "void func_8003DE14(RECT *rect, s32 count) {"),
            ("    StoreImage((s32 *)rect, src_buf);\n", "    StoreImage(rect, (u32 *)src_buf);\n"),
            ("    LoadImage((s32)rect, (s32)src_buf);\n", "    LoadImage(rect, (u32 *)src_buf);\n"),
            ("LoadImage((s32)rect, ((s32)dst_buf + j) - j);", "LoadImage(rect, (u32 *)(((s32)dst_buf + j) - j));"),
            ("                s32 new_y = ((u16 *)rect)[1] + ((u16 *)rect)[3];\n",
             "                /* FAKE: unsigned halfword reads (lhu); rect->y + rect->h scores 2 */\n"
             "                s32 new_y = (u16)rect->y + (u16)rect->h;\n"),
        ])
        for k, m in enumerate("xywh"):
            b = b.replace("((u16 *)rect)[%d]" % k, "rect->%s" % m).replace("rect[%d]" % k, "rect->%s" % m)
        return b
    t = in_fn(t, "void func_8003DE14(s16 *rect, s32 count) {", de14)
    for fn in ("void func_8003E0E0(void) {", "void func_8003E120(void) {"):
        t = in_fn(t, fn, lambda b: members(rep(b, [("    s16 buf[4];\n", "    RECT buf;\n"),
                                                   ("    func_8003DE14(buf, ", "    func_8003DE14(&buf, ")]), "buf", "."))
    return t

RECTS_3AB48 = [("D_800A327C", "{ 0x80, 3, 0, 0, 0x40, 0, 0, 1 }", "{ 0x380, 0, 0x40, 0x100 }"),
               ("D_800A3284", "{ 0xe0, 3, 0xff, 1, 0x10, 0, 1, 0 }", "{ 0x3E0, 0x1FF, 0x10, 1 }"),
               ("D_800A3294", "{ 0xc0, 3, 0x80, 1, 0x40, 0, 0x16, 0 }", "{ 0x3C0, 0x180, 0x40, 0x16 }"),
               ("D_800A329C", "{ 0xc0, 3, 0xff, 1, 0x10, 0, 1, 0 }", "{ 0x3C0, 0x1FF, 0x10, 1 }"),
               ("D_800A32A4", "{ 0x80, 3, 0x7f, 1, 0x40, 0, 0x7f, 0 }", "{ 0x380, 0x17F, 0x40, 0x7F }"),
               ("D_800A32AC", "{ 0x80, 3, 0xff, 1, 0x20, 0, 1, 0 }", "{ 0x380, 0x1FF, 0x20, 1 }")]

def s3AB48(t):
    for sym, old, new in RECTS_3AB48:
        t = rep(t, [("u8 %s[8] = %s;" % (sym, old), "RECT %s = %s;" % (sym, new)),
                    ("extern u8 %s[8];\n" % sym, "extern RECT %s;\n" % sym)])
    t = rep(t, [
        ("    u8 r1[8], r2[8];\n    s32 s0;\n", "    RECT r1, r2;\n    s32 s0;\n"),
        ("    u8 r1[8], r2[8], r3[8], r4[8];\n", "    RECT r1, r2, r3, r4;\n"),
        ("    LoadImage((s32)r1, (s32)(arg1 + 0x40));\n", "    LoadImage(&r1, (u32 *)(arg1 + 0x40));\n"),
        ("    LoadImage((s32)r2, (s32)(arg1 + 0x14));\n", "    LoadImage(&r2, (u32 *)(arg1 + 0x14));\n"),
        ("    LoadImage((s32)r1, (s32)(p + 0x40));\n", "    LoadImage(&r1, (u32 *)(p + 0x40));\n"),
        ("    LoadImage((s32)r2, (s32)(p + 0x14));\n", "    LoadImage(&r2, (u32 *)(p + 0x14));\n"),
        ("    LoadImage((s32)r3, (s32)((u8 *)arg1 + 0x60));\n", "    LoadImage(&r3, (u32 *)((u8 *)arg1 + 0x60));\n"),
        ("    LoadImage((s32)r4, (s32)((u8 *)arg1 + 0x14));\n", "    LoadImage(&r4, (u32 *)((u8 *)arg1 + 0x14));\n"),
    ])
    for r, d in (("r1", "D_800A327C"), ("r2", "D_800A3284"), ("r1", "D_800A3294"), ("r2", "D_800A329C"),
                 ("r3", "D_800A32A4"), ("r4", "D_800A32AC")):
        t = rep(t, [("    __builtin_memcpy(%s, %s, 8);\n" % (r, d), "    %s = %s;\n" % (r, d))])
    return t

def s51268(t):
    def cb8(b):
        b = rep(b, [("  typedef struct\n  {\n    s16 sp10;\n    s16 sp12;\n    s16 sp14;\n    s16 sp16;\n  } SLocal;\n  SLocal s;\n",
                     "  RECT s;\n")])
        for a, c in (("sp10", "x"), ("sp12", "y"), ("sp14", "w"), ("sp16", "h")):
            b = b.replace("s.%s" % a, "s.%s" % c)
        return rep(b, [("LoadImage(&s.x, new_var);", "LoadImage(&s, (u32 *)new_var);"),
                       ("LoadImage(&s.x, new_var + 0x1DC00);", "LoadImage(&s, (u32 *)(new_var + 0x1DC00));")])
    t = in_fn(t, "s32 func_80060CB8(s32 arg0, s32 arg1)", cb8)
    return rep(t, [
        ("extern void LoadImage(u8 *, s32);\n", ""),
        ("extern void ClearImage(s32, s32, s32, s32);\n", ""),
        ("extern u8 D_800A32D8[8];\n", "extern RECT D_800A32D8;\n"),
        ("    u8 rect[8];\n    s32 v0;\n", "    RECT rect;\n    s32 v0;\n"),
        ("    __builtin_memcpy(rect, D_800A32D8, 8);\n", "    rect = D_800A32D8;\n"),
        ("    ClearImage((s32)rect, 0, 0, 0);\n    DrawSync(0);\n    LoadImage(rect, temp_s3 + 0x14);\n",
         "    ClearImage(&rect, 0, 0, 0);\n    DrawSync(0);\n    LoadImage(&rect, (u32 *)(temp_s3 + 0x14));\n"),
        ("extern u8 D_800A32E0[8];\n", "extern RECT D_800A32E0;\n"),
        ("    u8 rect[8];\n    SetDispMask(0);\n", "    RECT rect;\n    SetDispMask(0);\n"),
        ("    __builtin_memcpy(rect, D_800A32E0, 8);\n    ClearImage((s32)rect, 0, 0, 0);\n",
         "    rect = D_800A32E0;\n    ClearImage(&rect, 0, 0, 0);\n"),
    ])

def s5ED34(t):
    t = rep(t, [
        ("extern void LoadImage(u8 *, s32);\n", ""),
        ("extern u8 D_800A32EC[8];\n", "extern RECT D_800A32EC;\n"),
        ("    __builtin_memcpy(&rect, D_800A32EC, 8);\n", "    rect = D_800A32EC;\n"),
        ("typedef struct {\n    s16 x;\n    s16 y;\n    s16 w;\n    s16 h;\n} Rect_8006ECF4;\nextern Rect_8006ECF4 D_800A32F4;\n",
         "extern RECT D_800A32F4;\n"),
        ("    Rect_8006ECF4 rectbuf;\n", "    RECT rectbuf;\n"),
        ("            LoadImage((s32)&rectbuf, a2);\n", "            LoadImage(&rectbuf, (u32 *)a2);\n"),
        ("        u8 *vram; /* several values of one kind: player i's VRAM rect row,\n"
         "                   * D_800A35A8->unk_7C + (i << 6), computed at the top of the\n",
         "        RECT *vram; /* several values of one kind: player i's VRAM rect row,\n"
         "                   * D_800A35A8->unk_7C + i * 8, computed at the top of the\n"),
        ("        vram = D_800A35A8->unk_7C;\n        vram += i << 6;\n",
         "        /* FAKE: base and step as two statements, here and in the locked-slot arm;\n"
         "         * `vram = D_800A35A8->unk_7C + i * 8` scores 28 */\n"
         "        vram = D_800A35A8->unk_7C;\n        vram += i * 8;\n"),
        ("                    vram = D_800A35A8->unk_7C;\n                    vram += i << 6;\n",
         "                    vram = D_800A35A8->unk_7C;\n                    vram += i * 8;\n"),
    ])
    assert t.count("LoadImage(vram + id * 8, *tim);") == 3
    t = t.replace("LoadImage(vram + id * 8, *tim);", "LoadImage(&vram[id], (u32 *)*tim);")
    def r4(b):
        b = members(rep(b, [("    s16 rect[4];\n", "    RECT rect;\n")]), "rect", ".")
        return b.replace("LoadImage(rect, data);", "LoadImage(&rect, (u32 *)data);") \
                .replace("LoadImage(rect, s3);", "LoadImage(&rect, (u32 *)s3);") \
                .replace("LoadImage(rect, s3 + 0x59400);", "LoadImage(&rect, (u32 *)(s3 + 0x59400));")
    t = in_fn(t, "void func_8006E8CC(Unk8006E950Head *a0) {", r4)
    return in_fn(t, "void func_8006E950(s32 a0, Unk8006E950Head *a1) {", r4)

def s64FD8(t):
    return rep(t, [
        ("extern void LoadImage(s32, s32);\n", ""),
        ("typedef struct {\n    s16 x, y, w, h;\n} Rect77D94;\nextern Rect77D94 D_800A32FC;\n", "extern RECT D_800A32FC;\n"),
        ("    Rect77D94 rect;\n", "    RECT rect;\n"),
        ("                LoadImage((s32)&rect, *img + 0x220);\n", "                LoadImage(&rect, (u32 *)(*img + 0x220));\n"),
    ])

def s6CF8(t):
    t = rep(t, [
        ("typedef struct {\n    s16 x;\n    s16 y;\n    s16 w;\n    s16 h;\n} Rect;\n\n", ""),
        ("extern Rect D_800A30D4;\n", "extern RECT D_800A30D4;\n"),
        ("    Rect rect;\n", "    RECT rect;\n"),
        ("extern void ClearImage(void *, s32, s32, s32);\n", ""),
        ("extern void LoadImage(u8 *, u8 *);\n", ""),
        ("extern u32 g_gpu_clear_rect;\n", "extern RECT g_gpu_clear_rect;\n"),
    ])
    assert t.count("LoadImage((u8 *)&rect, arg0 + 0x14);") == 2
    return t.replace("LoadImage((u8 *)&rect, arg0 + 0x14);", "LoadImage(&rect, (u32 *)(arg0 + 0x14));")

def s31D3C(t):
    t = rep(t, [("extern void LoadImage(s32, s32);\n", "")])
    def ac8(b):
        b = members(rep(b, [("  s16 rect[4];\n", "  RECT rect;\n")]), "rect", ".")
        return rep(b, [("StoreImage(rect, var_s1);", "StoreImage(&rect, (u32 *)var_s1);")])
    t = in_fn(t, "void func_80041AC8(Unk80045878Obj *arg0)", ac8)
    return in_fn(t, "void func_80041BF4(s32 a0, s32 a1, s32 a2)", lambda b: rep(b, [
        ("  /* FAKE: oversized locals object - rect[0..3] is the live LoadImage RECT and\n"
         "     rect[4..7] is the unwritten tail, mechanism:",
         "  /* FAKE: oversized locals object - rect[0] is the live LoadImage RECT and\n"
         "     rect[1] is the unwritten tail, mechanism:"),
        ("     fully-written form (rect[4]) gives frame 80, so no fully-written locals\n"
         "     set can produce target's 88.  ALIGN8 plus this frame's fixed 8-byte\n"
         "     phantom slot make the declared size recoverable only as a RANGE: rect[5]\n"
         "     through rect[8] are all byte-identical here - [8] is chosen.",
         "     fully-written form (one RECT) gives frame 80, so no fully-written locals\n"
         "     set can produce target's 88.  ALIGN8 plus this frame's fixed 8-byte\n"
         "     phantom slot make the declared size recoverable only as a RANGE: 10\n"
         "     through 16 bytes are all byte-identical here - two RECTs are chosen."),
        ("  s16 rect[8];\n", "  RECT rect[2];\n"),
        ("    rect[0] = x + xoff;\n    rect[1] = (*(((u16 *) tbl) + 1)) + yoff;\n    rect[2] = 0x10;\n    rect[3] = 1;\n"
         "    LoadImage((s32)rect, (s32)((u8 *)&g_gpu_store_buf + off));\n",
         "    rect[0].x = x + xoff;\n    rect[0].y = (*(((u16 *) tbl) + 1)) + yoff;\n    rect[0].w = 0x10;\n    rect[0].h = 1;\n"
         "    LoadImage(&rect[0], (u32 *)((u8 *)&g_gpu_store_buf + off));\n"),
        ("    func_80048A7C(rect[0], rect[1], 0x10, r, g, b);\n", "    func_80048A7C(rect[0].x, rect[0].y, 0x10, r, g, b);\n"),
    ]))

def s368E4(t):
    def fx(calls):
        def f(b):
            b = members(rep(b, [("    s16 rect[4];\n", "    RECT rect;\n")]), "rect", ".")
            return rep(b, calls)
        return f
    t = in_fn(t, "void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {", fx([
        ("LoadImage(rect, (s32 *)(arg0 + 4));", "LoadImage(&rect, (u32 *)(arg0 + 4));"),
        ("LoadImage(rect, (s32 *)buf);", "LoadImage(&rect, (u32 *)buf);"),
        ("LoadImage(rect, (s32 *)p_alt);", "LoadImage(&rect, (u32 *)p_alt);")]))
    t = in_fn(t, "void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {", fx([
        ("LoadImage(rect, (s32)buf);", "LoadImage(&rect, (u32 *)buf);"),
        ("LoadImage(rect, (s32)arg0);", "LoadImage(&rect, (u32 *)arg0);")]))
    return in_fn(t, "void func_80048864(s32 mode, s32 sx, s32 sy, s32 w, s32 mr, s32 mg, s32 mb, s32 dx, s32 dy) {", fx([
        ("StoreImage(rect, buf);", "StoreImage(&rect, (u32 *)buf);"),
        ("LoadImage(rect, out);", "LoadImage(&rect, (u32 *)out);")]))

def game(g):
    return rep(g, [
        (" * - unk_7C: per player a row of VRAM RECTs (64 bytes) func_80070F78 hands LoadImage as the\n",
         " * - unk_7C: per player a row of eight VRAM RECTs func_80070F78 hands LoadImage as the\n"),
        ("    u8 *unk_7C;\n", "    RECT *unk_7C;\n"),
    ])

def bb2(b):
    return rep(b, [("extern void func_8003DE14(s16 *, s32);\n", "extern void func_8003DE14(RECT *, s32);\n")])

STEPS = {"2B344.c": s2B344, "3AB48.c": s3AB48, "51268.c": s51268, "5ED34.c": s5ED34, "64FD8.c": s64FD8,
         "6CF8.c": s6CF8, "31D3C.c": s31D3C, "368E4.c": s368E4, "ext.c": ext, "sys.c": sysc,
         "libgpu.h": libgpu, "game.h": game, "bb2.h": bb2}

# ---------------------------------------------------------------- B4 sweep of the moved bodies (measured)
def sweep(out):
    # func_800602AC: arg1 also held the second TIM's address; its own local is byte-identical
    out["3AB48.c"] = in_fn(out["3AB48.c"], "void func_800602AC(s32 arg0, s32 *arg1) {", lambda b: rep(b, [
        ("    u8 *p;\n", "    u8 *p;\n    u8 *q;\n"),
        ("    arg1 = (s32 *)arg1[1];\n", "    q = (u8 *)arg1[1];\n"),
        ("    LoadImage(&r3, (u32 *)((u8 *)arg1 + 0x60));\n", "    LoadImage(&r3, (u32 *)(q + 0x60));\n"),
        ("    LoadImage(&r4, (u32 *)((u8 *)arg1 + 0x14));\n", "    LoadImage(&r4, (u32 *)(q + 0x14));\n")]))
    # func_8006E950: the s0_addr alias, the s2 / s0 constant holders, their (s16) casts and the v0 temp
    # are byte-identical as plain C
    out["5ED34.c"] = in_fn(out["5ED34.c"], "void func_8006E950(s32 a0, Unk8006E950Head *a1) {", lambda b: rep(b, [
        ("    s32 s2;\n    s32 s3;\n    s32 s0;\n    s32 s0_addr;\n    s32 v0;\n", "    s32 s3;\n"),
        ("    s0_addr = a0;\n    game_FrameLoop();\n    v0 = func_80036EA8(2, s0_addr);\n    cdrom_StartRead(v0, (s32)a1);\n",
         "    game_FrameLoop();\n    cdrom_StartRead(func_80036EA8(2, a0), (s32)a1);\n"),
        ("    s2 = 0x280;\n", ""),
        ("    s3 = a1->unk_08;\n    s0 = 0x1DC;\n", "    s3 = a1->unk_08;\n"),
        ("    rect.x = (s16)s2;\n    rect.y = 0;\n    rect.w = 0x180;\n    rect.h = (s16)s0;\n",
         "    rect.x = 0x280;\n    rect.y = 0;\n    rect.w = 0x180;\n    rect.h = 0x1DC;\n"),
        ("    rect.x = (s16)s2;\n    rect.y = (s16)s0;\n", "    rect.x = 0x280;\n    rect.y = 0x1DC;\n")]))
    # func_8006E10C: two carried holders, labelled
    out["51268.c"] = in_fn(out["51268.c"], "s32 func_8006E10C(void) {", lambda b: rep(b, [
        ("    s32 temp_s3 = D_800A3500;\n",
         "    s32 temp_s3 = D_800A3500; /* FAKE: read at entry and held in $s3 across the calls; read at the LoadImage call: score 9 (frame 48 for 56) */\n"),
        ("    s32 a0v;\n",
         "    s32 a0v; /* FAKE: the per-branch 2 (li a0,2 in each arm's delay slot); the literal at the call: score 9 */\n")]))
    return out

# ---------------------------------------------------------------- rev-fimg1's fixes (measured identical)
def rev1(out):
    out["2B344.c"] = in_fn(out["2B344.c"], "void func_8003DE14(RECT *rect, s32 count) {", lambda b: rep(b, [
        ("            {\n"
         "                /* FAKE: unsigned halfword reads (lhu); rect->y + rect->h scores 2 */\n"
         "                s32 new_y = (u16)rect->y + (u16)rect->h;\n"
         "                rect->y = new_y;\n"
         "                if ((s16)new_y >= 0x200) {\n"
         "                    rect->y = saved_y;\n"
         "                    rect->x += rect->w;\n"
         "                }\n"
         "            }\n",
         "            rect->y += rect->h;\n"
         "            if (rect->y >= 0x200) {\n"
         "                rect->y = saved_y;\n"
         "                rect->x += rect->w;\n"
         "            }\n")]))
    out["368E4.c"] = in_fn(out["368E4.c"], "void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {", lambda b: rep(b, [
        ("void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, s32 arg4) {\n    u16 arg4_lo = *(u16 *)&arg4;\n",
         "void func_800482C8(u8 *arg0, s16 arg1, s16 arg2, s16 arg3, u16 arg4) {\n"),
        ("    s16 buf[512];\n", "    u16 buf[512];\n"),
        ("    rect.y = arg4_lo;\n", "    rect.y = arg4;\n"),
        ("        math_GrayscaleRgb555((s32)p_alt, rect.w, (s32)buf);\n", "        math_GrayscaleRgb555((u16 *)p_alt, rect.w, buf);\n")]))
    out["368E4.c"] = in_fn(out["368E4.c"], "void func_800484A0(u8 *arg0, s16 arg1, s16 arg2) {", lambda b: rep(b, [
        ("    s16 buf[512];\n", "    u16 buf[512];\n"),
        ("        math_GrayscaleRgb555((s32)arg0, rect.w, (s32)buf);\n", "        math_GrayscaleRgb555((u16 *)arg0, rect.w, buf);\n")]))
    out["31D3C.c"] = in_fn(out["31D3C.c"], "void func_80041BF4(s32 a0, s32 a1, s32 a2)", lambda b: rep(b, [
        ("    rect[0].y = (*(((u16 *) tbl) + 1)) + yoff;\n", "    rect[0].y = tbl[1] + yoff;\n")]))
    out["ext.c"] = in_fn(out["ext.c"], "u16 LoadTPage(u32 *a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {", lambda b: rep(b, [
        ("u16 LoadTPage(u32 *a0, s32 mode, s32 a2, s32 a3, s32 texpage, s32 width, s32 clut) {",
         "u16 LoadTPage(u32 *pix, s32 tp, s32 abr, s32 x, s32 y, s32 w, s32 h) {"),
        ("    buf.x = a3;\n    buf.h = clut;\n    buf.y = texpage;\n    switch (mode) {\n",
         "    buf.x = x;\n    buf.h = h;\n    buf.y = y;\n    switch (tp) {\n"),
        ("        buf.w = width / 4;\n", "        buf.w = w / 4;\n"),
        ("        buf.w = width / 2;\n", "        buf.w = w / 2;\n"),
        ("        buf.w = width;\n", "        buf.w = w;\n"),
        ("    LoadImage(&buf, a0);\n    return GetTPage(mode, a2, a3, texpage) & 0xFFFF;",
         "    LoadImage(&buf, pix);\n    return GetTPage(tp, abr, x, y);")]))
    out["51268.c"] = in_fn(out["51268.c"], "s32 func_8006E10C(void) {", lambda b: rep(b, [
        ("    do { ff0 = 0xF0; } while (0); /* FAKE: loop notes fence sched1's constant-sink so the li stays at the jal */\n",
         "    do { ff0 = 0xF0; } while (0); /* FAKE: loop notes fence sched1's constant-sink so the li stays at the jal; plain `ff0 = 0xF0;`: score 13 */\n")]))
    return out

def write():
    base = {}
    if CHAIN:
        import fclose1
        fclose1.BASE_REV = BASE_REV
        base = fclose1.write()
    os.makedirs(OUT, exist_ok=True)
    out = {}
    for n, p in FILES.items():
        out[n] = STEPS[n](base[n] if n in base else show(p))
    out = sweep(out)
    out = rev1(out)
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fimg1")
