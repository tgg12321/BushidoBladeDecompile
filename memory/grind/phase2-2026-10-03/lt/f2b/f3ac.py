#!/usr/bin/env python3
# Growth of the 3AB48 batch (on f3ab.py): 64FD8 func_80074B18's D_SEL rectangle rows; (more parts below).
# usage: f3ac.py [opt=<name>,...]   writes tmp/p2/f3ac/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f3ab as P
sys.argv = _argv
OUT = "tmp/p2/f3ac/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = P.sub1
fn = P.fn

def rects(t, g):
    # D_SEL's unk_3C holds 12-byte rectangle rows {x, y, w, h, r, g, b} (data: 72 00 00 00 ac 00 01 00 ...),
    # the layout of Rec_8006C21C (MOD.BIN's unk_44 rows)
    rec = ("/* The 12-byte records MOD.BIN's unk_44 points at (Unk8006919CRec), drawn by func_8006C21C. */\n"
           "typedef struct {\n    s16 x, y, w, h;\n    u8 r, g, b, pad;\n} Rec_8006C21C;\n\n")
    g = sub1(g, rec, "")
    newrec = ("/* 12-byte rectangle rows {x, y, w, h, r, g, b}: MOD.BIN's unk_44 (func_8006C21C draws them) and\n"
              " * D_SEL.BIN's unk_3C (func_80074B18). */\n"
              "typedef struct {\n    s16 x, y, w, h;\n    u8 r, g, b, pad;\n} Rec_8006C21C;\n\n")
    anchor = "/* SEL.BIN / SEL1.BIN / SEL2.BIN (resource files 3-5)"
    g = sub1(g, anchor, newrec + anchor)
    g = sub1(g, "    u8 *unk_3C;\n} Unk80076FF8Rec;", "    Rec_8006C21C *unk_3C;\n} Unk80076FF8Rec;")
    g = sub1(g, "head: unk_14..unk_38, the ten lists func_80076FF8 relocates (unk_20 is indexed by the round count\n * SelWork.f65); unk_3C, the bytes func_80074B18 reads. */",
             "head: unk_14..unk_38, the ten lists func_80076FF8 relocates (unk_20 is indexed by the round count\n * SelWork.f65); unk_3C, the rectangle rows func_80074B18 draws. */")
    def b(x):
        x = sub1(x, "    u8 *t;\n", "    Rec_8006C21C *t;\n")
        for a, c in (("t[8]", "t->r"), ("t[9]", "t->g"), ("t[0xA]", "t->b"),
                     ("*(u16 *)(t + 4)", "t->w"), ("*(u16 *)(t + 6)", "t->h"),
                     ("*(u16 *)(t + 0)", "t->x"), ("*(u16 *)(t + 2)", "t->y")):
            x = x.replace(a, c)
        x = sub1(x, "            t += 0xC;\n", "            t++;\n")
        return x
    t = fn(t, "func_80074B18", b)
    return t, g

INNER_OLD = """typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[5];
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ s32 quads[3][4];
} Func8003F6D8Inner;"""

INNER_NEW = """typedef struct {
    /* 0x00 */ s32 count;
    /* 0x04 */ s32 *objs[3];
    /* 0x10 */ s32 unk10;
    /* 0x14 */ s32 unk14;
    /* 0x18 */ s16 pairs[3][16];
    /* 0x78 */ s32 unk78[3];
    /* 0x84 */ SVECTOR quads[3][2];
} Func8003F6D8Inner;"""

FECC_NEW = """void func_8003FECC(u8 *a0, SceneRec *rec, s16 *a2) {
    Func8003F6D8Inner *in = &rec->inner;
    s32 n = in->count;
    s16 id = a2[0];

    if (id != -2) {
        do {
            a2++;
            in->objs[n] = (s32 *)(a0 + 0x94 + id * 0x68);
            in->quads[n][0].vx = *a2++;
            in->quads[n][0].vy = *a2++;
            in->quads[n][0].vz = *a2++;
            in->quads[n][1].vx = *a2++;
            in->quads[n][1].vy = *a2++;
            in->quads[n][1].vz = *a2++;
            in->unk78[n] = *a2++;
            n++;
            id = *a2;
        } while (id != -2);
    }
    in->count = n;
    in->unk14 = n;
}
"""

def inner(w):
    w = sub1(w, INNER_OLD, INNER_NEW)
    w = sub1(w, "extern void gte_SetMatrixRotTransIR(s32 *, s32 *, s16 *);", "extern void gte_SetMatrixRotTransIR(s32 *, SVECTOR *, s16 *);")
    w = sub1(w, "gte_SetMatrixRotTransIR(obj, in->quads[j], in->pairs[j]);", "gte_SetMatrixRotTransIR(obj, &in->quads[j][0], in->pairs[j]);")
    w = sub1(w, "gte_SetMatrixRotTransIR(obj, in->quads[j] + 2, in->pairs[j] + 8);", "gte_SetMatrixRotTransIR(obj, &in->quads[j][1], in->pairs[j] + 8);")
    w = sub1(w, "    rec->inner.objs[4] = 0;\n", "    rec->inner.unk14 = 0;\n")
    w = sub1(w, "    rec->quad.unkC = &rec->inner.objs[3];\n", "    rec->quad.unkC = &rec->inner.unk10;\n")
    w = sub1(w, "   the address of the record's inner.objs[3]. */", "   the address of the record's inner.unk10. */")
    if "nofecc" not in OPT:
        i = w.index("void func_8003FECC(s32 *a0, s32 *a1, s16 *a2)\n{")
        j = w.index("\n}\n", i) + 3
        w = w[:i] + FECC_NEW + w[j:]
        w = sub1(w, "void func_8003FECC(s32 *a0, s32 *a1, s16 *a2);", "void func_8003FECC(u8 *a0, SceneRec *rec, s16 *a2);")
        w = sub1(w, "func_8003FECC((s32 *)arg0, (s32 *)rec, cmds);", "func_8003FECC(arg0, rec, cmds);")
    return w

PACK = """/* The VAB pack func_8005C2A8 loads (MOD.BIN's head unk_00 points at one; g_vab_rec_ptr keeps the
   loaded ones): four words, the first three file-relative offsets func_8005C2A8 turns into addresses
   in place. unk_00: the u32 key-event table func_8005C6D0 reads; unk_04: the VabHdr snd_VabOpen opens;
   unk_08: the body snd_VabOpen transfers; unk_0C: the body's size in SPU memory. */
typedef struct {
    s32 unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
} Unk8005C2A8Pack;

"""
T = "Unk8005C2A8Pack"

def vab(a, s, e, g):
    anchor = "/* The head of the resource files func_8006E950 loads."
    g = sub1(g, anchor, PACK + anchor)
    a = sub1(a, "extern s32 *g_vab_rec_ptr[];\n", "extern %s *g_vab_rec_ptr[];\n" % T)
    a = sub1(a, "extern s32 func_8005C2A8(s32 *, s16, s32);", "extern s32 func_8005C2A8(%s *, s16, s32);" % T)
    s = sub1(s, "extern s32 func_8005C2A8(s32 *, s16, s32);", "extern s32 func_8005C2A8(%s *, s16, s32);" % T)
    e = sub1(e, "extern s32 func_8005C2A8(s32 *, s16, s32);", "extern s32 func_8005C2A8(%s *, s16, s32);" % T)
    s = sub1(s, "func_8005C2A8((s32 *)a0[0], 1, a0[1]);", "func_8005C2A8((%s *)a0[0], 1, a0[1]);" % T)
    e = sub1(e, "func_8005C2A8((s32 *)a0[0], 1, a0[1]);", "func_8005C2A8((%s *)a0[0], 1, a0[1]);" % T)
    a = sub1(a, "func_8005C2A8((s32 *)arg0, 0,", "func_8005C2A8((%s *)arg0, 0," % T)
    a = sub1(a, "func_8005C2A8((s32 *)arg0, 8,", "func_8005C2A8((%s *)arg0, 8," % T)
    a = sub1(a, "func_8005C2A8((s32 *)(arg0 + ret), 4,", "func_8005C2A8((%s *)(arg0 + ret), 4," % T)
    a = sub1(a, "func_8005C2A8((s32 *)a0, 9,", "func_8005C2A8((%s *)a0, 9," % T)
    a = sub1(a, "func_8005C2A8((s32 *)loc.ent[i].off, ", "func_8005C2A8((%s *)loc.ent[i].off, " % T)
    def c2a8(b):
        b = sub1(b, "s32 func_8005C2A8(s32 *hdr, s16 vabid, s32 arg2) {", "s32 func_8005C2A8(%s *hdr, s16 vabid, s32 arg2) {" % T)
        b = sub1(b, "    hdr[0] += (s32) hdr;\n    hdr[1] += (s32) hdr;\n    hdr[2] += (s32) hdr;\n",
                 "    hdr->unk_00 += (s32) hdr;\n    hdr->unk_04 += (s32) hdr;\n    hdr->unk_08 += (s32) hdr;\n")
        b = sub1(b, "        D_800A3408 += hdr[3];\n", "        D_800A3408 += hdr->unk_0C;\n")
        b = sub1(b, "        return hdr[2] - (s32) hdr;\n", "        return hdr->unk_08 - (s32) hdr;\n")
        b = sub1(b, "g_vab_rec_ptr[i][3]", "g_vab_rec_ptr[i]->unk_0C")
        return b
    a = fn(a, "func_8005C2A8", c2a8)
    a = sub1(a, "extern s32 snd_VabOpen(s32 *, s16);", "extern s32 snd_VabOpen(%s *, s16);" % T)
    def vo(b):
        b = sub1(b, "s32 snd_VabOpen(s32 *a0, s16 a1) {", "s32 snd_VabOpen(%s *a0, s16 a1) {" % T)
        b = sub1(b, "    SsVabOpenHeadSticky(a0[1], a1, g_vab_sticky_sbaddr);\n    *(s32 *)(a0[1] + 8) = a1;\n    return (s16)SsVabTransBody(a0[2], a1);\n",
                 "    SsVabOpenHeadSticky(a0->unk_04, a1, g_vab_sticky_sbaddr);\n    ((VabHdr *)a0->unk_04)->id = a1;\n    return (s16)SsVabTransBody(a0->unk_08, a1);\n")
        return b
    a = fn(a, "snd_VabOpen", vo)
    def mv(b):
        b = b.replace("g_vab_rec_ptr[arg1][3]", "g_vab_rec_ptr[arg1]->unk_0C").replace("g_vab_rec_ptr[arg1][1]", "g_vab_rec_ptr[arg1]->unk_04")
        return b
    a = fn(a, "snd_MoveVabBody", mv)
    a = a.replace("g_vab_rec_ptr[0][3]", "g_vab_rec_ptr[0]->unk_0C").replace("g_vab_rec_ptr[order[j]][3]", "g_vab_rec_ptr[order[j]]->unk_0C")
    a = sub1(a, "ev = &((u32 *)g_vab_rec_ptr[vab][0])[p[1]];", "ev = &((u32 *)g_vab_rec_ptr[vab]->unk_00)[p[1]];")
    def fake(b):
        b = sub1(b, "    s32 **base;\n    s32 **p;\n    s32 *v;\n    s32 *vv;\n", "    %s **base;\n    %s **p;\n    %s *v;\n" % (T, T, T))
        b = sub1(b, "        v = (s32 *)((u8 *)v + arg0);\n", "        v = (%s *)((u8 *)v + arg0);\n" % T)
        b = sub1(b, "        *v = *v + arg0;\n", "        v->unk_00 = v->unk_00 + arg0;\n")
        b = sub1(b, "        vv = *p;\n        vv[1] = vv[1] + arg0;\n", "        (*p)->unk_04 = (*p)->unk_04 + arg0;\n")
        b = sub1(b, "SsVabFakeHead((*p)[1], idx,", "SsVabFakeHead((*p)->unk_04, idx,")
        return b
    a = fn(a, "snd_VabFakeOpen", fake)
    a = fn(a, "snd_Init", lambda b: sub1(b, "    s32 **p2;\n", "    %s **p2;\n" % T))
    a = fn(a, "snd_Quit", lambda b: sub1(b, "    s32 **v1;\n", "    %s **v1;\n" % T))
    a = fn(a, "func_8005B72C", lambda b: sub1(b, "    s32 **s1;\n", "    %s **s1;\n" % T))
    a = sub1(a, "    s32 **s3 = g_vab_rec_ptr;\n", "    %s **s3 = g_vab_rec_ptr;\n" % T)
    left = [l.strip() for l in a.split(NL) if re.search(r"g_vab_rec_ptr\[[^]]*\]\[", l)]
    for l in left:
        print("LEFT", l)
    return a, s, e, g

# ---------------------------------------------------------------- rev-f3ab re-review fixes (all measured IDENTICAL by the reviewer)
def rev2_head(g, m, e):
    """Unk8006E950Head.unk_00 is the VAB pack; the MOD / SEL relocators read the head and lists by member."""
    g = sub1(g, "typedef struct {\n    s32 *unk_00;\n    s32 unk_04;\n", "typedef struct {\n    Unk8005C2A8Pack *unk_00;\n    s32 unk_04;\n")
    def m919c(b):
        b = sub1(b, "s32 func_8006919C(s32 *a0) {", "s32 func_8006919C(Unk8006919CRec *a0) {")
        b = sub1(b, "    s32 *p = &a0[5];\n", "    s32 **p = &a0->unk_14;\n")
        b = sub1(b, "        func_8006920C(a0, *p);\n", "        func_8006920C((s32 *)a0, (s32)*p);\n")
        b = sub1(b, "    func_8005C2A8((Unk8005C2A8Pack *)a0[0], 1, a0[1]);\n    return a0[1];\n",
                 "    func_8005C2A8(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);\n    return a0->unk_00.unk_04;\n")
        return b
    m = fn(m, "func_8006919C", m919c)
    e = sub1(e, "    D_800A356C = func_8006EA28((s32 *)D_800A356C);\n", "    D_800A356C = func_8006EA28((Unk8006EA28Rec *)D_800A356C);\n")
    def ea28(b):
        b = sub1(b, "s32 func_8006EA28(s32 *a0) {", "s32 func_8006EA28(Unk8006EA28Rec *a0) {")
        for k, mem in zip(range(21, 30), ["unk_54", "unk_58", "unk_5C", "unk_60", "unk_64", "unk_68", "unk_6C", "unk_70", "unk_74"]):
            b = sub1(b, "    func_8006920C(a0, a0[%d]);\n" % k, "    func_8006920C((s32 *)a0, (s32)a0->%s);\n" % mem)
        b = sub1(b, "    func_8005C2A8((Unk8005C2A8Pack *)a0[0], 1, a0[1]);\n    return a0[1];\n",
                 "    func_8005C2A8(a0->unk_00.unk_00, 1, a0->unk_00.unk_04);\n    return a0->unk_00.unk_04;\n")
        return b
    e = fn(e, "func_8006EA28", ea28)
    return g, m, e

CLV_OLD = """    Unk8005C2A8Pack **s3 = g_vab_rec_ptr;
    s32 *s2 = g_vab_vb_sbaddr;
    u8 *s0 = g_vab_id_list;
    u8 *s1 = (u8 *)((s32)s0 + 3);
    do {
        SsVabClose(*s0);
        s3[*s0] = 0;
        s2[*s0] = 0;
        s0++;
    } while ((s32)s0 < (s32)s1);
"""
CLV_NEW = """    s32 i;
    for (i = 0; i < 3; i++) {
        SsVabClose(g_vab_id_list[i]);
        g_vab_rec_ptr[g_vab_id_list[i]] = 0;
        g_vab_vb_sbaddr[g_vab_id_list[i]] = 0;
    }
"""

def rev2_3AB48(a):
    a = fn(a, "snd_CloseListedVabs", lambda b: sub1(b, CLV_OLD, CLV_NEW))
    def s04(b):
        b = sub1(b, "    s32 p;\n", "")
        return sub1(b, """    p = s->unk2C;
    s->unk4 = *(s32 *)(((Unk800469C4Hdr *)p)->unk4 + p);
    p = s->unk2C;
    s->unk2 = *(u16 *)(((Unk800469C4Hdr *)p)->unk8 + p);
""", """    s->unk4 = *(s32 *)(((Unk800469C4Hdr *)s->unk2C)->unk4 + s->unk2C);
    s->unk2 = *(u16 *)(((Unk800469C4Hdr *)s->unk2C)->unk8 + s->unk2C);
""")
    a = fn(a, "func_80054604", s04)
    def b9fc(b):
        b = sub1(b, "    s32 s1;\n", "    s32 task;\n    s32 size;\n")
        b = sub1(b, "    s1 = func_80036EA8(2, 8);\n", "    task = func_80036EA8(2, 8);\n")
        b = sub1(b, "    cdrom_StartRead(s1, a0);\n    s1 = cdrom_GetFileSize(s1);\n", "    cdrom_StartRead(task, a0);\n    size = cdrom_GetFileSize(task);\n")
        return sub1(b, "a0 + s1);", "a0 + size);")
    a = fn(a, "func_8005B9FC", b9fc)
    a = fn(a, "func_8005BD30", lambda b: sub1(b, "            u8 byte = g_vab_id_list[i & 0xFF];\n            snd_VabFakeOpen(arg0, byte);\n",
                                           "            snd_VabFakeOpen(arg0, g_vab_id_list[i & 0xFF]);\n"))
    return a

def rev2_2B344(s):
    i = s.index("typedef struct {\n    /* 0x00 */ s32 count;\n    /* 0x04 */ s32 *objs[3];")
    j = s.index("} Func8003F6D8Inner;", i)
    j = s.index("\n", j) + 1
    inner = s[i:j]
    s = s[:i] + s[j:]
    i = s.index("/* The 16 bytes func_8003FA24 fills at SceneRec +0x04")
    j = s.index("} Scene;\n", i) + len("} Scene;\n")
    blk = s[i:j]
    s = s[:i] + s[j:]
    k = s.index("void func_8003F62C(s32 *a0) {")
    s = s[:k] + inner + "\n" + blk + "\n" + s[k:]
    s = sub1(s, """        s32 off = i * 0xD0 + 8;
        Func8003F6D8Inner *in = (Func8003F6D8Inner *)((u8 *)arg0 + off + 0x1C);
""", "        Func8003F6D8Inner *in = &arg0->recs[i].inner;\n")
    s = sub1(s, "void func_8003F6D8(s16 *arg0) {", "void func_8003F6D8(Scene *arg0) {")
    s = sub1(s, "    for (i = 0; i < arg0[0]; i++) {\n        Func8003F6D8Inner *in = &arg0", "    for (i = 0; i < arg0->count; i++) {\n        Func8003F6D8Inner *in = &arg0")
    s = sub1(s, "    func_8003F6D8(s0);\n", "    func_8003F6D8((Scene *)s0);\n")
    def f824(b):
        b = sub1(b, "    s16 c;\n", "")
        return sub1(b, "        c = *cmds;\n        if (c != -2) {\n", "        if (*cmds != -2) {\n")
    return fn(s, "func_8003F824", f824)

# ---------------------------------------------------------------- rev-f3ab round 3 (the two newly moved keys)
F62C_OLD = """void func_8003F62C(s32 *a0) {
    s16 *s0;
    s32 *s1 = a0;
    s0 = (s16 *)s1[9];
    if (s0 == 0) return;
    if (s0[3]) {
        func_8004016C(*(s16 *)((u8 *)s1 + 4));
        func_8003F824((u8 *)s1, 0);
    }
    if (s0[1]) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8((Scene *)s0);
    func_8001924C(&s0[0x20C], s0[0]);
    if (s0[1]) {
        func_80040068((u8 *)s0);
        s0[1] = 0;
    }
}
"""
F62C_NEW = """void func_8003F62C(s32 *a0) {
    Scene *s0;
    s0 = (Scene *)a0[9];
    if (s0 == 0) return;
    if (s0->unk6) {
        func_8004016C(*(s16 *)((u8 *)a0 + 4));
        func_8003F824((u8 *)a0, 0);
    }
    if (s0->unk2) {
        func_8004001C((u8 *)s0);
    }
    func_8003F6D8(s0);
    func_8001924C((s16 *)s0->quads, s0->count);
    if (s0->unk2) {
        func_80040068((u8 *)s0);
        s0->unk2 = 0;
    }
}
"""

def rev3_2B344(s):
    return sub1(s, F62C_OLD, F62C_NEW)

def rev3_5ED34(t):
    def e534(b):
        b = sub1(b, """    {
        s32 b = D_800A3588[0];
        s32 c = D_800A358C[0];
        D_8009BC7C[D_8009BC40[c][b].value] |= 4;
    }
""", """    D_8009BC7C[D_8009BC40[D_800A358C[0]][D_800A3588[0]].value] |= 4;
""")
        b = sub1(b, """        s32 b = D_800A3588[1];
        s32 c = D_800A358C[1];
        D_8009BC7C[D_8009BC40[c][b].value] |= 4;
""", """        D_8009BC7C[D_8009BC40[D_800A358C[1]][D_800A3588[1]].value] |= 4;
""")
        if "keepvalue" not in OPT:
            b = sub1(b, "    s16 i;\n    u8 value;\n", "    s16 i;\n")
            b = sub1(b, """        value = D_8009BC7C[i] & 0xFA;
        D_8009BC7C[i] = value;
        if (arg3 & (1 << i)) {
            D_8009BC7C[i] = value | 1;
        }
""", """        D_8009BC7C[i] &= 0xFA;
        if (arg3 & (1 << i)) {
            D_8009BC7C[i] |= 1;
        }
""")
        return b
    return fn(t, "func_8006E534", e534)

def stagehdr_up(g):
    """Unk800469C4Hdr (f3ab.py) above Unk800EFAE8Ctrl's comment, not between the comment and the struct."""
    i = g.index("/* The stage data a stage loads (func_800469C4")
    j = g.index("} Unk800469C4Hdr;\n\n") + len("} Unk800469C4Hdr;\n\n")
    blk = g[i:j]
    g = g[:i] + g[j:]
    anchor = "/* Stage/match control block at 0x800EFAE8 (0x4C bytes)."
    return sub1(g, anchor, blk + anchor)

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(P.write())
    out["64FD8.c"], out["game.h"] = rects(out["64FD8.c"], out["game.h"])
    out["2B344.c"] = inner(out["2B344.c"])
    if "novab" not in OPT:
        out["3AB48.c"], out["51268.c"], out["5ED34.c"], out["game.h"] = vab(out["3AB48.c"], out["51268.c"], out["5ED34.c"], out["game.h"])
    if "u16rec" in OPT:
        out["game.h"] = sub1(out["game.h"], "    s16 x, y, w, h;\n    u8 r, g, b, pad;\n} Rec_8006C21C;", "    u16 x, y, w, h;\n    u8 r, g, b, pad;\n} Rec_8006C21C;")
    out["game.h"] = stagehdr_up(out["game.h"])
    out["game.h"], out["51268.c"], out["5ED34.c"] = rev2_head(out["game.h"], out["51268.c"], out["5ED34.c"])
    out["3AB48.c"] = rev2_3AB48(out["3AB48.c"])
    out["2B344.c"] = rev2_2B344(out["2B344.c"])
    out["2B344.c"] = rev3_2B344(out["2B344.c"])
    out["5ED34.c"] = rev3_5ED34(out["5ED34.c"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f3ac", sorted(OPT))
