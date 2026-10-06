#!/usr/bin/env python3
# FZZ in the unowned TUs, batch 2: the object table (g_file_data_buf, eight 0x34-byte records 6CF8
# builds and 87A0 steps) and its input record (2B344 func_8003FA24's init).
# usage: fzz2.py [opt=<name>,...]   writes tmp/p2/fzz2/ (scratch only)
# opt=split: all but func_8003FA24 (func_80017D84 keeps its u8 *a0 param and its raw a0 reads)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "fdesc"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import fdesc1 as D
sys.argv = _argv
OUT = "tmp/p2/fzz2/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1, fn = D.sub1, D.fn
BASE_REV = os.environ.get("BASE_REV", "HEAD")
FILES = {"6CF8.c": "src/main/6CF8.c", "87A0.c": "src/main/87A0.c", "2B344.c": "src/main/2B344.c",
         "bb2.h": "include/bb2.h", "game.h": "include/game.h"}

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True, encoding="utf-8").stdout

def subs(b, pairs):
    for p in pairs:
        if len(p) == 3 and p[2] is None:
            assert p[0] in b, p[0][:70]
            b = b.replace(p[0], p[1])
        else:
            b = sub1(b, p[0], p[1])
    return b

def cut_types(t):
    """6CF8's Func80017A44Record / Func80017848Edge / Func80017A44Output / Func80017A44Input"""
    blks = {}
    for name in ("Func80017A44Record", "Func80017848Edge", "Func80017A44Output", "Func80017A44Input"):
        j = t.index("} %s;\n" % name) + len("} %s;\n" % name)
        i = t.rindex("typedef struct {", 0, j)
        blks[name] = t[i:j]
        t = t[:i] + t[j:].lstrip(NL)
    return t, blks

OUTPUT = """/* 6CF8's object record (sizeof = 0x34): g_file_data_buf holds eight, func_80017D84 fills a free one
 * from a Func80017A44Input and func_80017A44 builds its node / edge graph; 87A0 steps it
 * (func_8001924C by the scene quad's id). points: the input's point table, 0 = free (obj_Clear);
 * count: its node count; flags: the input's flags; records / edges: the node and edge tables
 * (edges right after the count 0x40-byte nodes); matrix: a copy of the input's matrix. */
typedef struct {
    SVECTOR *points;
    s16 count;
    s16 edge_count;
    s32 flags;
    Func80017A44Record *records;
    Func80017848Edge *edges;
    MATRIX matrix;
} Func80017A44Output;
"""
INPUT = """/* func_80017D84's argument (2B344 func_8003FA24 builds it on its stack): the point table and its
 * count, the flags, the group list, the object's matrix and the buffer the node / edge tables go to. */
typedef struct {
    s16 count;
    s16 flags;
    SVECTOR *points;
    s16 *groups;
    MATRIX *matrix;
    u8 *buf;
} Func80017A44Input;
"""

def game(g, blks):
    anchor = "typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;\n"
    add = blks["Func80017A44Record"] + NL + blks["Func80017848Edge"] + NL + OUTPUT + NL + INPUT + NL
    return sub1(g, anchor, add + anchor)

def bb2(b):
    b = sub1(b, "extern u8 g_file_data_buf[];\n", "extern Func80017A44Output g_file_data_buf[8];\n")
    if "split" in OPT:
        return b
    return sub1(b, "extern s32 func_80017D84(u8 *);\n", "extern s32 func_80017D84(Func80017A44Input *);\n")

def s6CF8(t):
    t, blks = cut_types(t)
    t = sub1(t, "typedef struct { s32 v[8]; } ObjBlock;\n", "")
    t = fn(t, "obj_ClearAll", lambda b: subs(b, [
        ("    for (i = 0x16C; i >= 0; i -= 0x34) {\n        *(s32 *)(g_file_data_buf + i) = 0;\n    }\n",
         "    /* FAKE: the table walked by byte offset: the target's one induction variable is the\n"
         "       record's offset (`li v0,364` ... `addiu v0,v0,-52`); indexed by record, loop.c keeps\n"
         "       the index beside its scaled copy: score 4. */\n"
         "    for (i = 7 * sizeof(Func80017A44Output); i >= 0; i -= sizeof(Func80017A44Output)) {\n"
         "        ((Func80017A44Output *)((u8 *)g_file_data_buf + i))->points = 0;\n    }\n")]))
    if "split" in OPT:
        # without func_8003FA24's call (worker 2's batch 3) the prototype keeps u8 *a0: one boundary
        # conversion at entry, the body typed as in the full batch
        t = fn(t, "func_80017D84", lambda b: subs(b, [
            ("s32 func_80017D84(u8 *a0) {\n    u8 *p;\n    s32 i;\n    s32 c;\n",
             "s32 func_80017D84(u8 *a0) {\n    Func80017A44Input *in = (Func80017A44Input *)a0;\n    Func80017A44Output *p;\n    s32 i;\n    u8 *c;\n"),
            ("        if (*(s32 *)p == 0) break;\n        p += 0x34;\n", "        if (p->points == 0) break;\n        p++;\n"),
            ("    *(u16 *)(p + 4) = *(u16 *)a0;\n", "    p->count = in->count;\n"),
            ("    *(s32 *)p = *(s32 *)(a0 + 4);\n", "    p->points = in->points;\n"),
            ("    *(ObjBlock *)(p + 0x14) = **(ObjBlock **)(a0 + 0xC);\n", "    p->matrix = *in->matrix;\n"),
            ("    *(s32 *)(p + 8) = *(s16 *)(a0 + 2);\n", "    p->flags = in->flags;\n"),
            ("    c = *(s32 *)(a0 + 0x10);\n", "    c = in->buf;\n"),
            ("    *(s16 *)(p + 6) = 0;\n", "    p->edge_count = 0;\n"),
            ("    *(s32 *)(p + 0xC) = c;\n    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);\n",
             "    p->records = (Func80017A44Record *)c;\n    p->edges = (Func80017848Edge *)((Func80017A44Record *)c + p->count);\n"),
            ("    func_80017A44(a0, p);\n", "    func_80017A44(in, p);\n")]))
    else:
      t = fn(t, "func_80017D84", lambda b: subs(b, [
        ("s32 func_80017D84(u8 *a0) {\n    u8 *p;\n", "s32 func_80017D84(Func80017A44Input *a0) {\n    Func80017A44Output *p;\n"),
        ("        if (*(s32 *)p == 0) break;\n        p += 0x34;\n", "        if (p->points == 0) break;\n        p++;\n"),
        ("    *(u16 *)(p + 4) = *(u16 *)a0;\n", "    p->count = a0->count;\n"),
        ("    *(s32 *)p = *(s32 *)(a0 + 4);\n", "    p->points = a0->points;\n"),
        ("    *(ObjBlock *)(p + 0x14) = **(ObjBlock **)(a0 + 0xC);\n", "    p->matrix = *a0->matrix;\n"),
        ("    *(s32 *)(p + 8) = *(s16 *)(a0 + 2);\n", "    p->flags = a0->flags;\n"),
        ("    s32 c;\n", "    u8 *c;\n"),
        ("    c = *(s32 *)(a0 + 0x10);\n", "    c = a0->buf;\n"),
        ("    *(s16 *)(p + 6) = 0;\n", "    p->edge_count = 0;\n"),
        ("    *(s32 *)(p + 0xC) = c;\n    *(s32 *)(p + 0x10) = c + (*(s16 *)(p + 4) << 6);\n",
         "    p->records = (Func80017A44Record *)c;\n    p->edges = (Func80017848Edge *)((Func80017A44Record *)c + p->count);\n")]))
    t = fn(t, "obj_Clear", lambda b: sub1(b, "    *(s32 *)(g_file_data_buf + a0 * 52) = 0;\n", "    g_file_data_buf[a0].points = 0;\n"))
    t = fn(t, "obj_UpdatePosition", lambda b: subs(b, [
        ("    u8 *ptr = g_file_data_buf + a0 * 52;\n    s32 c = *(s32 *)(ptr + 0xC) + a1;\n    *(s32 *)(ptr + 0xC) = c;\n    *(s32 *)(ptr + 0x10) = c + (*(s16 *)(ptr + 4) << 6);\n",
         "    Func80017A44Output *ptr = &g_file_data_buf[a0];\n\n    ptr->records = (Func80017A44Record *)((u8 *)ptr->records + a1);\n    ptr->edges = (Func80017848Edge *)(ptr->records + ptr->count);\n")]))
    t = fn(t, "obj_AddValue", lambda b: subs(b, [
        ("    s32 *ptr = (s32 *)(g_file_data_buf + a0 * 52);\n    *ptr = *ptr + a1;\n",
         "    Func80017A44Output *ptr = &g_file_data_buf[a0];\n    ptr->points = (SVECTOR *)((u8 *)ptr->points + a1);\n")]))
    return t, blks

def s2B344(t):
    def fa24(b):
        b = subs(b, [("""    struct SceneObjInit {
        s16 count;
        s16 flags;
        u8 *points;
        s16 *groups;
        void *matrix;
        u8 *point_end;
        s32 unk14;
        s32 unk18;
        s32 unk1C;
    } init;
""", "    /* FAKE: frame layout (oversized live object): func_80017D84's argument plus an unwritten\n"
     "       three-word tail; the argument alone gives a frame 8 short (0x48 for 0x50): score 16. */\n"
     "    struct {\n        Func80017A44Input in;\n        s32 tail[3];\n    } init;\n"),
            ("    init.points = cur;\n", "    init.points = (SVECTOR *)cur;\n"),
            ("    init.point_end = cur;\n", "    init.buf = cur;\n"),
            ("    rec->unk0 = func_80017D84((u8 *)&init);\n    obj->unk60 = init.point_end;\n",
             "    rec->unk0 = func_80017D84(&init);\n    obj->unk60 = init.buf;\n")])
        b = re.sub(r"\binit\.(?!in\b)", "init.in.", b)
        return b.replace("func_80017D84(&init)", "func_80017D84(&init.in)")
    return fn(t, "func_8003FA24", fa24)

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {k: show(v) for k, v in FILES.items()}
    out["6CF8.c"], blks = s6CF8(out["6CF8.c"])
    out["game.h"] = game(out["game.h"], blks)
    out["bb2.h"] = bb2(out["bb2.h"])
    if "split" not in OPT:
        out["2B344.c"] = s2B344(out["2B344.c"])
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote fzz2", sorted(OPT))
