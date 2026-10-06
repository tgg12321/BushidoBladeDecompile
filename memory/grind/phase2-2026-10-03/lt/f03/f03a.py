#!/usr/bin/env python3
# F03 batch a, on top of 2e7120591 (Q113 committed as f3f430cf9; later commits touch no build input): 51268's resource root, the
# MOD.BIN header func_80068F70 loads (file 2 of func_80036EA8's group 2), gets its layout type
# Unk8006919CRec in game.h, after the head every resource file shares (Unk8006E950Head). The reads
# through the draw context's word 1 (the root; the context itself stays a word array, P3 round 2)
# become member reads. Rec_8006C21C moves to game.h unchanged (Unk8006919CRec.unk_44 points at it).
# The D_800A34FC-held reads are f03b (with F19).
# usage: f03a.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f03/{51268.c,game.h,bb2.h} (scratch only); with opt=..., tmp/p2/f03/opt/
#   opt L<func> : the one-local form in that body (default: the per-site view)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f01"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import q113 as Q
sys.argv = _argv
OUT = "tmp/p2/f03/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = Q.sub1
fn = Q.D2.fn

BASE_REV = "2e7120591"

# ---------------------------------------------------------------- game.h
TYPES = """
/* The head of the resource files func_8006E950 loads. By g_cd_file_table's sizes, file 2 of
 * func_80036EA8's group 2 is MOD.BIN, 3 SEL.BIN, 4 / 5 SEL1 / SEL2.BIN, 6 D_SEL.BIN, 0x32 NAR.BIN.
 * Each file starts with a list of file-relative offsets ending in -1, which func_8006E440 turns
 * into addresses. The first five mean the same in every file: func_8006919C / func_8006EA28 pass
 * unk_00 and unk_04 to the VAB loader func_8005C2A8 (func_80076FF8 / func_80077D10, for D_SEL /
 * NAR, make no such call, and those files' unk_00..unk_08 are equal), func_8006E950 loads the
 * image at unk_08 into VRAM, and func_8006E8CC loads the 640x32 strip at unk_0C or unk_10. The
 * four per-file relocators (func_8006919C, func_8006EA28, func_80076FF8, func_80077D10) return
 * unk_04, where the caller's func_8006E49C buffer starts. The words after the head differ per file. */
typedef struct {
    s32 *unk_00;
    s32 unk_04;
    s32 unk_08;
    s32 unk_0C;
    s32 unk_10;
} Unk8006E950Head;

/* The 12-byte records MOD.BIN's unk_44 points at (Unk8006919CRec), drawn by func_8006C21C. */
typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;

/* MOD.BIN (resource file 2), the root 51268's func_80068F70 loads at its work area + 0x58; it keeps
 * it in D_800A34FC's word 9, and func_8006E390 copies that into word 1 of the draw context. After
 * the head:
 * - unk_14..unk_40: the twelve lists func_8006919C relocates (func_8006920C: entries up to a 0
 *   word, -1 entries skipped). Most entries are sprite sheets, a 12-byte Unk8009B0E0Record header
 *   followed by its 8-byte Unk8009B400Record cells (hence the readers' `+ 0xC`). The lists stay
 *   s32 *: their entries go into the s32 header words of the draw descriptors, which keep that
 *   type until the descriptors are unified.
 * - unk_44: the Rec_8006C21C rows func_8006C21C draws; unk_48: the points func_8006BEC4 reads.
 * The -1 ending func_8006E440's offset list follows at +0x4C; no reader touches it. */
typedef struct {
    Unk8006E950Head unk_00;
    s32 *unk_14;
    s32 *unk_18;
    s32 *unk_1C;
    s32 *unk_20;
    s32 *unk_24;
    s32 *unk_28;
    s32 *unk_2C;
    s32 *unk_30;
    s32 *unk_34;
    s32 *unk_38;
    s32 *unk_3C;
    s32 *unk_40;
    Rec_8006C21C *unk_44;
    Vec2s16 *unk_48;
} Unk8006919CRec;
"""

def game(g):
    a = "typedef struct Vec2s16 { s16 x; s16 y; } Vec2s16;\n"
    return sub1(g, a, a + TYPES)

# ---------------------------------------------------------------- 51268.c
R = "Unk8006919CRec"

def site(b, old, new, n=1):
    return sub1(b, old, new, n)

def v(off):          # the per-site view of the context's word 1 (s32 *arg0)
    return "((%s *)arg0[1])->unk_%02X" % (R, off)

def b69AE4(b):
    return site(b, "qbase = *(s32 **)(arg0[1] + 0x34);", "qbase = %s;" % v(0x34))

def b69E18(b):
    b = site(b, "    s32 ptr;\n", "    s32 *ptr;\n")
    b = site(b, "ptr = *(s32 *)(*(s32 *)(arg0 + 4) + 0x14);", "ptr = (*(%s **)(arg0 + 4))->unk_14;" % R)
    b = site(b, "s.p0 = (s32 *)*(s32 *)ptr;", "s.p0 = (s32 *)ptr[0];")
    b = site(b, "p0 = *(s32 *)(ptr + 4);", "p0 = ptr[1];")
    b = site(b, "p0 = *(s32 *)(ptr + 8);", "p0 = ptr[2];")
    return b

def b69F80(b):
    return site(b, "ptr = *(s32 **)(arg0[1] + 0x1C);", "ptr = %s;" % v(0x1C))

def b6A3CC(b):
    return site(b, "*(s32 *)(*(s32 *)(*(s32 *)((u8 *)arg0 + 4) + 0x1C) + 0x10);",
                "%s[4];" % v(0x1C))

def b6A494(b):
    return site(b, "*(s32 *)(*(s32 *)(*(s32 *)((u8 *)arg0 + 4) + 0x1C) + 0x24);",
                "%s[9];" % v(0x1C))

def b6A564(b):
    b = site(b, "    u8 *obj2;\n", "    %s *root;\n" % R)
    b = site(b, "obj2 = *(u8 **)(arg0 + 4);", "root = *(%s **)(arg0 + 4);" % R)
    b = site(b, "tbl = *(s32 **)(obj2 + 0x1C);", "tbl = root->unk_1C;")
    return b

def b6A880(b):
    b = site(b, """       sprite-sheet table of the MOD.BIN root: +0x18 (option rows; first sheet,
       the row loop and row 7), +0x40 (counter frames), +0x24 twice (icon
       frames at [8 + frame], then FT4 frames at [frame]). */""",
             """       sprite-sheet list of the MOD.BIN root: unk_18 (option rows; first sheet,
       the row loop and row 7), unk_40 (counter frames), unk_24 twice (icon
       frames at [8 + frame], then FT4 frames at [frame]). */""")
    if "L8006A880" in OPT:
        b = site(b, "    s32 *sheets;\n", "    s32 *sheets;\n    %s *root;\n" % R)
        b = site(b, "    sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x18);\n    row_mask",
                 "    root = *(%s **)(arg0 + 4);\n    sheets = root->unk_18;\n    row_mask" % R)
        for off, n in ((0x18, 1), (0x40, 1), (0x24, 2)):
            b = site(b, "sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x%X);" % off, "sheets = root->unk_%02X;" % off, n)
    else:
        for off, n in ((0x18, 2), (0x40, 1), (0x24, 2)):
            b = site(b, "sheets = *(s32 **)(*(s32 *)(arg0 + 4) + 0x%X);" % off,
                     "sheets = (*(%s **)(arg0 + 4))->unk_%02X;" % (R, off), n)
    return b

def local_s32(b, decl_after, sites):
    # one `root` local: declared after decl_after, set where the first view stood
    b = site(b, decl_after, decl_after + "    %s *root;\n" % R)
    first = True
    for old, off, n in sites:
        if first:
            assert b.count(old) == n, (old, n)
            b = b.replace("    " + old, "    root = (%s *)arg0[1];\n    %s" % (R, old), 1)
            b = site(b, old, old.replace("*(s32 **)(arg0[1] + 0x%X)" % off, "root->unk_%02X" % off), n)
            first = False
        else:
            b = site(b, old, old.replace("*(s32 **)(arg0[1] + 0x%X)" % off, "root->unk_%02X" % off), n)
    return b

def per_site(b, sites):
    for old, off, n in sites:
        b = site(b, old, old.replace("*(s32 **)(arg0[1] + 0x%X)" % off, v(off)), n)
    return b

def b6B120(b):
    s = [("tbl = *(s32 **)(arg0[1] + 0x28);", 0x28, 2)]
    return local_s32(b, "    s32 *tbl;\n", s) if "L8006B120" in OPT else per_site(b, s)

def b6BB68(b):
    s = [("q = *(s32 **)(arg0[1] + 0x2C);", 0x2C, 1), ("q = *(s32 **)(arg0[1] + 0x28);", 0x28, 1)]
    return local_s32(b, "    s32 *q;\n", s) if "L8006BB68" in OPT else per_site(b, s)

def b6C21C(b):
    s = [("table = *(s32 **)(arg0[1] + 0x30);", 0x30, 4)]
    return local_s32(b, "    s32 *table;\n", s) if "L8006C21C" in OPT else per_site(b, s)

def b6CFBC(b):
    return per_site(b, [("table = *(s32 **)(arg0[1] + 0x30);", 0x30, 1)])

def b6D3DC(b):
    return per_site(b, [("q = *(s32 **)(arg0[1] + 0x38);", 0x38, 1)])

def b6DD94(b):
    return per_site(b, [("q = *(s32 **)(arg0[1] + 0x3C);", 0x3C, 1)])

def b6A1A0(b):
    return site(b, "ptr = *(s32 **)(arg0[1] + 0x1C);", "ptr = %s;" % v(0x1C))

BODIES = [("func_80069AE4", b69AE4), ("func_80069E18", b69E18), ("func_80069F80", b69F80),
          ("func_8006A1A0", b6A1A0), ("func_8006A3CC", b6A3CC), ("func_8006A494", b6A494),
          ("func_8006A564", b6A564), ("func_8006A880", b6A880), ("func_8006B120", b6B120),
          ("func_8006BB68", b6BB68), ("func_8006C21C", b6C21C), ("func_8006CFBC", b6CFBC),
          ("func_8006D3DC", b6D3DC), ("func_8006DD94", b6DD94)]
FUNCS = [f for f, _ in BODIES]

def src(s):
    s = sub1(s, """typedef struct {
    s16 x, y, w, h;
    u8 r, g, b, pad;
} Rec_8006C21C;


""", "")
    for f, g in BODIES:
        s = fn(s, f, g)
    return s

def base():
    def show(p):
        return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                              text=True, encoding="utf-8").stdout
    return show("src/main/51268.c"), show("include/game.h"), show("include/bb2.h"), show("src/main/64FD8.c")

def write():
    d = OUT + ("opt/" if OPT else "")
    os.makedirs(d, exist_ok=True)
    s0, g0, h0, t = base()
    s = src(s0); g = game(g0); h = h0
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

if __name__ == "__main__":
    s, g, h, t = write()
    if "measure" in sys.argv[1:]:
        fs = [a for a in sys.argv[1:] if a.startswith("func_")] or FUNCS
        Q.D2.D1.measure(s, g, h, t, fs)
    print("wrote f03a", sorted(OPT))
