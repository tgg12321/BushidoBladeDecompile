#!/usr/bin/env python3
# F01 batch d-2, on top of F01d-1 (f01d1.py's output): func_80061FAC / func_800620B8 / func_80063084 /
# func_800646E8 take their own Unk1F8000B8Union members (func_80061FAC's is a bare SVECTOR).
# usage: f01d2.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f01d2/{51268.c,game.h,bb2.h,64FD8.c} (scratch only); with opt=..., tmp/p2/f01d2/opt/
import os, re, shutil, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f01d1 as D1
sys.argv = _argv
OUT = "tmp/p2/f01d2/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = D1.sub1
fn = D1.fn

def base():
    s0, g0, h0 = D1.base()
    return D1.src(s0), D1.game(g0), h0, D1.f64(subprocess.run(["git", "show", D1.HEAD64 + ":src/main/64FD8.c"],
                                                          capture_output=True, check=True, text=True, encoding="utf-8").stdout)

LAYOUTS = """/* func_800620B8's layout of the 51268 work area (Unk1F8000B8Union.v800620B8). func_80061FAC, which
 * func_800620B8 calls before it touches the area, writes and consumes its SVECTOR at +0x00..+0x07.
 * SetTransMatrix is handed the address 0x14 below unk14, so unk14 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                /* no access here */
    s16 unk10;                     /* w */
    s16 unk12;                     /* h */
    VECTOR unk14;                  /* ApplyRotMatrixLV out */
    VECTOR unk24;                  /* ApplyRotMatrixLV in */
    SVECTOR unk34;                 /* RotTransPers in */
    s32 unk3C;                     /* RotTransPers' p */
    u8 unk40[4];                   /* no access */
    u32 unk44;                     /* func_80052C28's depth, the OT index */
} Unk1F8000B8_800620B8;

/* func_80063084's layout (Unk1F8000B8Union.v80063084). SetTransMatrix is handed the address 0x14
 * below unk14 (the area's base), so unk14 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x14];                /* no access */
    VECTOR unk14;                  /* ApplyRotMatrix out */
    SVECTOR unk24;                 /* ApplyRotMatrix in */
    SVECTOR unk2C;                 /* RotTransPers in */
    s32 unk34;                     /* RotTransPers' p */
    s32 unk38;                     /* fade */
    s32 unk3C;                     /* func_80052C28's depth, the OT index */
} Unk1F8000B8_80063084;

/* func_800646E8's layout (Unk1F8000B8Union.v800646E8). SetTransMatrix is handed the address 0x14
 * below unk18, so unk18 is read as a MATRIX's t[]. */
typedef struct {
    u8 unk00[0x10];                /* no access */
    s16 unk10;                     /* w */
    s16 unk12;                     /* h */
    s32 unk14;                     /* frame */
    VECTOR unk18;                  /* ApplyRotMatrixLV out */
    VECTOR unk28;                  /* ApplyRotMatrixLV in */
    SVECTOR unk38;                 /* RotTransPers in */
    s32 unk40;                     /* RotTransPers' p */
    POLY_FT4 *unk44;               /* the end of the quads */
    u32 unk48[16];                 /* per kept quad, its OT index */
} Unk1F8000B8_800646E8;

"""

def game(g):
    a = "/* func_80065800's layout of the 51268 work area (Unk1F8000B8Union.v80065800)."
    g = sub1(g, a, LAYOUTS + a)
    g = sub1(g, """ * raw sizes the union. func_80065800 and the trio use their members; func_80061FAC, func_800620B8,
 * func_8006295C, func_80063084, func_80063E10 and func_800646E8 still convert D_800A34EC (later
 * batches). */
typedef union {
    u8 raw[0x400 - 0xB8];
    Unk1F8000B8_80065800 v80065800;
""", """ * raw sizes the union. func_80061FAC's is one SVECTOR. func_8006295C and func_80063E10 still convert
 * D_800A34EC (their work-area base is staged through a POLY_FT4 * local). */
typedef union {
    u8 raw[0x400 - 0xB8];
    SVECTOR v80061FAC;
    Unk1F8000B8_800620B8 v800620B8;
    Unk1F8000B8_80063084 v80063084;
    Unk1F8000B8_800646E8 v800646E8;
    Unk1F8000B8_80065800 v80065800;
""")
    return g

def holder(b, struct, member, fields):
    b = sub1(b, "    u8 *base;\n", "    %s *base;\n" % struct)
    b = sub1(b, "    base = (u8 *)D_800A34EC;\n", "    base = &D_800A34EC->%s;\n" % member)
    for old, new in fields:
        b = sub1(b, old, new)
    left = [l for l in b.split(NL) if "(base + " in l and not l.strip().startswith(("/*", "*")) and "here). Spelled" not in l]
    assert not left, left
    return b

def b61FAC(b):
    return sub1(b, "    SVECTOR *dest = (SVECTOR *)D_800A34EC;\n", "    SVECTOR *dest = &D_800A34EC->v80061FAC;\n")

def b620B8(b):
    b = holder(b, "Unk1F8000B8_800620B8", "v800620B8", [
        ("w = (s16 *)(base + 0x10);", "w = &base->unk10;"),
        ("h = (s16 *)(base + 0x12);", "h = &base->unk12;"),
        ("tv = (VECTOR *)(base + 0x14);", "tv = &base->unk14;"),
        ("v = (VECTOR *)(base + 0x24);", "v = &base->unk24;"),
        ("sv = (SVECTOR *)(base + 0x34);", "sv = &base->unk34;"),
        ("interp = (s32 *)(base + 0x3C);", "interp = &base->unk3C;"),
        ("z = (u32 *)(base + 0x44);", "z = &base->unk44;")])
    if "tvbase" in OPT:
        b = sub1(b, "        SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));\n", "        SetTransMatrix((MATRIX *)base);\n")
    return b

def b63084(b):
    b = holder(b, "Unk1F8000B8_80063084", "v80063084", [
        ("tv = (VECTOR *)(base + 0x14);", "tv = &base->unk14;"),
        ("sv = (SVECTOR *)(base + 0x24);", "sv = &base->unk24;"),
        ("v = (SVECTOR *)(base + 0x2C);", "v = &base->unk2C;"),
        ("interp = (s32 *)(base + 0x34);", "interp = &base->unk34;"),
        ("fade = (s32 *)(base + 0x38);", "fade = &base->unk38;"),
        ("z = (s32 *)(base + 0x3C);", "z = &base->unk3C;")])
    if "tvbase" in OPT:
        b = sub1(b, "                SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));\n", "                SetTransMatrix((MATRIX *)base);\n")
    return b

def b646E8(b):
    b = holder(b, "Unk1F8000B8_800646E8", "v800646E8", [
        ("zbuf = (u32 *)(base + 0x48);", "zbuf = base->unk48;"),
        ("w = (s16 *)(base + 0x10);", "w = &base->unk10;"),
        ("h = (s16 *)(base + 0x12);", "h = &base->unk12;"),
        ("frame = (s32 *)(base + 0x14);", "frame = &base->unk14;"),
        ("trans = (VECTOR *)(base + 0x18);", "trans = &base->unk18;"),
        ("pos = (VECTOR *)(base + 0x28);", "pos = &base->unk28;"),
        ("sv = (SVECTOR *)(base + 0x38);", "sv = &base->unk38;"),
        ("p = (s32 *)(base + 0x40);", "p = &base->unk40;"),
        ("end = (POLY_FT4 **)(base + 0x44);", "end = &base->unk44;")])
    if "tvbase" in OPT:
        b = sub1(b, "            SetTransMatrix((MATRIX *)((u8 *)trans - 0x14));\n", "            SetTransMatrix((MATRIX *)((u8 *)base + 4));\n")
    return b

IDIOM = [
    # func_80063084 / func_800646E8 move here: their tv - 0x14 SetTransMatrix idioms are load-bearing
    # (abl: score 55 / 68), labelled as func_800620B8's was in F01b2; code text unchanged
    ("""                /* SetTransMatrix reads only m->t (+0x14): hand it the address
                   0x14 below tv so tv is loaded as the translation. Spelled
                   (MATRIX *)base, base stays live across the loops: +20 bytes. */
""", """                /* FAKE: SetTransMatrix reads only m->t (+0x14): hand it the address
                   0x14 below tv so tv is loaded as the translation. Spelled
                   (MATRIX *)base, base stays live across the loops: score 55, 666
                   insns for 661 (+20 bytes; a 104-byte frame for 96, base loaded into
                   $s7 for the target's $v0). */
"""),
    ("""            /* the MATRIX whose t[] is *trans: SetTransMatrix reads only m->t
               (base+0x10/0x12/0x14 hold w/h/frame -- there is no whole MATRIX
               here). Spelled (MATRIX *)(base + 4), base stays live across the
               loop: +24 bytes. */
""", """            /* FAKE: the MATRIX whose t[] is *trans: SetTransMatrix reads only m->t
               (base+0x10/0x12/0x14 hold w/h/frame -- there is no whole MATRIX
               here). Spelled (MATRIX *)((u8 *)base + 4), base stays live across
               the loop: score 68, 496 insns for 490 (+24 bytes; a 112-byte frame
               for 104, base loaded into $s4 for the target's $v0). */
""")]

def src(s):
    for a, b in IDIOM:
        s = sub1(s, a, b)
    s = fn(s, "func_80061FAC", b61FAC)
    s = fn(s, "func_800620B8", b620B8)
    s = fn(s, "func_80063084", b63084)
    s = fn(s, "func_800646E8", b646E8)
    return s

def write():
    d = OUT + ("opt/" if OPT else "")
    os.makedirs(d, exist_ok=True)
    s0, g0, h0, t = base()
    s = src(s0); g = game(g0); h = h0
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h), ("64FD8.c", t)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

if __name__ == "__main__":
    s, g, h, t = write()
    if "measure" in sys.argv[1:]:
        D1.measure(s, g, h, t, [a for a in sys.argv[1:] if a.startswith("func_")])
    print("wrote d2", sorted(OPT))
