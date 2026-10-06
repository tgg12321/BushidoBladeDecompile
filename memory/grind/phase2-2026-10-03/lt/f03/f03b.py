#!/usr/bin/env python3
# F03 batch b (F19 folded in), on top of f03a as committed (71073622e): 51268's work block D_800A34FC, the 0x34 bytes
# func_80068F70 reserves at func_8006E49C's return (D_800A3500 = block + 0x34), gets its layout
# type Unk800A34FCRec in game.h; the holder becomes a pointer to it and every site reads members.
# Its +0x24 word is the MOD.BIN root (Unk8006919CRec *, f03a).
# usage: f03b.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f03b/{51268.c,game.h,bb2.h} (scratch only); with opt=..., tmp/p2/f03b/opt/
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f03a as A
sys.argv = _argv
OUT = "tmp/p2/f03b/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = A.sub1

def fn(s, f, g):
    m = re.search(r"\n[A-Za-z0-9_]+[ *]+%s\([^;{]*\)\s*\{" % f, s)   # the definition, not a prototype
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return s[:i] + g(s[i:j]) + s[j:]

W = "D_800A34FC"

# ---------------------------------------------------------------- game.h
TYPE = """
/* 51268's work block: the 0x34 bytes func_80068F70 keeps at the start of func_8006E49C's
 * returned space (D_800A34FC; its arena cursor D_800A3500 moves past the block). func_80068F70
 * zeroes unk_0C / unk_10 / unk_12, sets both unk_28 halves to 5 and unk_30 to bit 0 of D_800A3524's
 * word 8 (func_80069120 compares and refreshes it), and stores the MOD.BIN root in unk_24.
 * - unk_0C: the s16 pair func_800692C0 steps (its arg2); the draw functions use unk_0C[0] / [1] as
 *   x / y offsets.
 * - unk_28: one s16 per player (func_8006C21C, func_8006CBD4, func_8006CCC8); func_8006CCC8 and
 *   func_8006CFBC also test the pair as one word (== 0x50005: both 5), as SelWork's f1C / f20.
 * No code touches the other bytes. */
typedef struct {
    u8 pad00[0xC];
    s16 unk_0C[2];
    s16 unk_10;
    s16 unk_12;
    u8 pad14[0x10];
    Unk8006919CRec *unk_24;
    union {
        s16 half[2];
        s32 word;
    } unk_28;
    u8 pad2C[4];
    u8 unk_30;
    u8 pad31[3];
} Unk800A34FCRec;
"""

def game(g):
    a = "} Unk8006919CRec;\n"
    return sub1(g, a, a + TYPE)

# ---------------------------------------------------------------- 51268.c
def b68F70(b):
    if "k_e49c" not in OPT:
        b = sub1(b, "    s32 *v0_e49c;\n", "    Unk800A34FCRec *v0_e49c;\n")
        b = sub1(b, "    v0_e49c = func_8006E49C(v0_efc, D_800A351C);\n    v0_e49c[9] = temp_s0;\n",
                 "    v0_e49c = (Unk800A34FCRec *)func_8006E49C(v0_efc, D_800A351C);\n"
                 "    v0_e49c->unk_24 = (Unk8006919CRec *)temp_s0;\n")
        b = sub1(b, "    D_800A34FC = (s32)v0_e49c;\n", "    D_800A34FC = v0_e49c;\n")
        if "k_add" not in OPT:
            b = sub1(b, "    D_800A3500 = (s32)v0_e49c + 0x34;\n", "    D_800A3500 = (s32)(v0_e49c + 1);\n")
    else:
        b = sub1(b, "    D_800A34FC = (s32)v0_e49c;\n", "    D_800A34FC = (Unk800A34FCRec *)v0_e49c;\n")
    if "v8" in OPT:   # s16 * holders of the two member pairs
        b = sub1(b, "            s32 p_34fc;\n", "            s16 *p_28;\n            s16 *p_0C;\n")
        b = sub1(b, "            p_34fc = D_800A34FC;\n",
                 "            p_28 = D_800A34FC->unk_28.half;\n            p_0C = D_800A34FC->unk_0C;\n")
        for old, new in (("*(s16 *)(p_34fc + 0x2A) = value;", "p_28[1] = value;"),
                         ("*(s16 *)(p_34fc + 0x28) = value;", "p_28[0] = value;"),
                         ("*(s16 *)(p_34fc + 0x12) = 0;", "D_800A34FC->unk_12 = 0;"),
                         ("*(s16 *)(p_34fc + 0x10) = 0;", "D_800A34FC->unk_10 = 0;"),
                         ("*(s16 *)(p_34fc + 0xE) = 0;", "p_0C[1] = 0;"),
                         ("*(s16 *)(p_34fc + 0xC) = 0;", "p_0C[0] = 0;")):
            b = sub1(b, old, new)
    elif "v6" in OPT:   # no holder: member stores straight through D_800A34FC
        b = sub1(b, "            s32 p_34fc;\n", "")
        b = sub1(b, "            p_34fc = D_800A34FC;\n", "")
        for old, new in (("*(s16 *)(p_34fc + 0x2A) = value;", "D_800A34FC->unk_28.half[1] = value;"),
                         ("*(s16 *)(p_34fc + 0x28) = value;", "D_800A34FC->unk_28.half[0] = value;"),
                         ("*(s16 *)(p_34fc + 0x12) = 0;", "D_800A34FC->unk_12 = 0;"),
                         ("*(s16 *)(p_34fc + 0x10) = 0;", "D_800A34FC->unk_10 = 0;"),
                         ("*(s16 *)(p_34fc + 0xE) = 0;", "D_800A34FC->unk_0C[1] = 0;"),
                         ("*(s16 *)(p_34fc + 0xC) = 0;", "D_800A34FC->unk_0C[0] = 0;")):
            b = sub1(b, old, new)
    elif "typed_p" in OPT:
        b = sub1(b, "            s32 p_34fc;\n", "            Unk800A34FCRec *p_34fc;\n")
        for old, new in (("*(s16 *)(p_34fc + 0x2A) = value;", "p_34fc->unk_28.half[1] = value;"),
                         ("*(s16 *)(p_34fc + 0x28) = value;", "p_34fc->unk_28.half[0] = value;"),
                         ("*(s16 *)(p_34fc + 0x12) = 0;", "p_34fc->unk_12 = 0;"),
                         ("*(s16 *)(p_34fc + 0x10) = 0;", "p_34fc->unk_10 = 0;"),
                         ("*(s16 *)(p_34fc + 0xE) = 0;", "p_34fc->unk_0C[1] = 0;"),
                         ("*(s16 *)(p_34fc + 0xC) = 0;", "p_34fc->unk_0C[0] = 0;")):
            b = sub1(b, old, new)
    elif "s32h" in OPT:   # the s32 holder with raw stores (as at HEAD)
        b = sub1(b, "            p_34fc = D_800A34FC;\n", "            p_34fc = (s32)D_800A34FC;\n")
    else:   # one s16 * holder per stored field, set where the work-block load stood
        r4 = "r4" in OPT   # pair holders for unk_28 / unk_0C, field holders for unk_10 / unk_12
        r7 = "r7" in OPT   # field holders for the pairs, unk_10 / unk_12 stored directly
        decl = ["p_2A", "p_28", "p_0E", "p_0C", "p_10", "p_12"]
        if r4:
            decl = ["p_28", "p_0C", "p_10", "p_12"]
        if r7:
            decl = ["p_2A", "p_28", "p_0E", "p_0C"]
        cm = ("            /* FAKE: one s16 * holder per stored work-block field, set before the\n"
              "               D_800A3524 store. The stores through them are plain `*p` (not\n"
              "               in-struct), so the D_800A3528 zero store and the D_800A34F8 update\n"
              "               keep their place; as member stores through an Unk800A34FCRec *\n"
              "               holder they sink below the block stores and the D_800A32C0 copy\n"
              "               loads (score 28); straight through D_800A34FC, 39. Partial sets:\n"
              "               pair holders for unk_28 / unk_0C with unk_10 / unk_12 direct, 8,\n"
              "               with field holders for those two, 4; only unk_0C as a pair, 2;\n"
              "               unk_10 / unk_12 stored direct, 4. */\n")
        b = sub1(b, "            s32 p_34fc;\n",
                 cm + "".join("            s16 *%s;\n" % d for d in decl))
        sets = {"p_2A": "&D_800A34FC->unk_28.half[1]", "p_28": "&D_800A34FC->unk_28.half[0]",
                "p_0E": "&D_800A34FC->unk_0C[1]", "p_0C": "&D_800A34FC->unk_0C[0]",
                "p_10": "&D_800A34FC->unk_10", "p_12": "&D_800A34FC->unk_12"}
        if r4:
            sets["p_28"] = "D_800A34FC->unk_28.half"
            sets["p_0C"] = "D_800A34FC->unk_0C"
        b = sub1(b, "            p_34fc = D_800A34FC;\n",
                 "".join("            %s = %s;\n" % (d, sets[d]) for d in decl))
        st = {"0x2A": "*p_2A = value;", "0x28": "*p_28 = value;", "0x12": "*p_12 = 0;",
              "0x10": "*p_10 = 0;", "0xE": "*p_0E = 0;", "0xC": "*p_0C = 0;"}
        if r4:
            st.update({"0x2A": "p_28[1] = value;", "0x28": "p_28[0] = value;",
                       "0xE": "p_0C[1] = 0;", "0xC": "p_0C[0] = 0;"})
        if r7:
            st.update({"0x12": "D_800A34FC->unk_12 = 0;", "0x10": "D_800A34FC->unk_10 = 0;"})
        for k, v in (("0x2A", "value"), ("0x28", "value"), ("0x12", "0"), ("0x10", "0"), ("0xE", "0"), ("0xC", "0")):
            b = sub1(b, "*(s16 *)(p_34fc + %s) = %s;" % (k, v), st[k])
    if "nob" in OPT:
        b = sub1(b, """            do { /* FAKE: block fence keeps li v0,5 at the join-block head so
                    reorg steals it into all three incoming jump delay slots */
            } while (0);
""", "")
    if "nos" in OPT:
        b = sub1(b, """            do { /* FAKE: sched fence keeps the D_800A3524 store adjacent to the
                    D_800A34FC load instead of sinking below the zero-stores */
            } while (0);
""", "")
    if "k_30" not in OPT:
        b = sub1(b, "*(s8 *)((u8 *)D_800A34FC + 0x30) = (s8)(((s32 *)D_800A3524)[8] & 1);",
                 "D_800A34FC->unk_30 = ((s32 *)D_800A3524)[8] & 1;" if "s8cast" not in OPT else
                 "D_800A34FC->unk_30 = (s8)(((s32 *)D_800A3524)[8] & 1);")
    return b

def b69120(b):
    b = sub1(b, "    u8 *v1 = (u8 *)D_800A34FC;\n", "    Unk800A34FCRec *v1 = D_800A34FC;\n")
    b = sub1(b, "    v1 = (u8 *)D_800A34FC;\n", "    v1 = D_800A34FC;\n")
    b = sub1(b, "if (v1[0x30] != (v0[8] & 1)) {", "if (v1->unk_30 != (v0[8] & 1)) {")
    b = sub1(b, "v1[0x30] = (u8)(v0[8] & 1);", "v1->unk_30 = v0[8] & 1;" if "u8cast" not in OPT else "v1->unk_30 = (u8)(v0[8] & 1);")
    return b

def xy(b):
    # the unk_0C pair reads and the func_800692C0 arguments
    b = b.replace("*(s16 *)(D_800A34FC + 0xC)", "D_800A34FC->unk_0C[0]")
    b = b.replace("*(s16 *)(D_800A34FC + 0xE)", "D_800A34FC->unk_0C[1]")
    b = b.replace("(s16 *)(D_800A34FC + 0xC)", "D_800A34FC->unk_0C")
    b = b.replace("func_800692C0(&sp10, 0, D_800A34FC + 0xC, D_800A350C)",
                  "func_800692C0(&sp10, 0, D_800A34FC->unk_0C, D_800A350C)")
    return b

def b6BD28(b):
    b = sub1(b, "sheets = *(s32 **)(*(s32 *)(D_800A34FC + 0x24) + 0x20);", "sheets = D_800A34FC->unk_24->unk_20;")
    b = sub1(b, "    /* sprite-sheet header pointers, two per arg0: the table that word +0x20 of\n"
                "       the block at *(D_800A34FC + 0x24) points to */",
             "    /* sprite-sheet header pointers, two per arg0: the unk_20 list of the\n"
             "       MOD.BIN root (D_800A34FC->unk_24) */")
    return b

def b6BEC4(b):
    return sub1(b, "pos = *(Vec2s16 **)(*(s32 *)(D_800A34FC + 0x24) + 0x48);", "pos = D_800A34FC->unk_24->unk_48;")

def b6C21C(b):
    b = sub1(b, "*(s16 *)(D_800A34FC + pl * 2 + 0x28)", "D_800A34FC->unk_28.half[pl]", 2)
    b = sub1(b, "recs = *(Rec_8006C21C **)(*(s32 *)(D_800A34FC + 0x24) + 0x44);", "recs = D_800A34FC->unk_24->unk_44;")
    b = sub1(b, "work = *(s16 *)(D_800A34FC + j * 2 + 0x28);", "work = D_800A34FC->unk_28.half[j];")
    return b

def b6CBD4(b):
    return sub1(b, "*(s16 *)((u8 *)D_800A34FC + (arg0 * 2) + 0x28)", "D_800A34FC->unk_28.half[arg0]")

def b6CCC8(b):
    b = sub1(b, "*(s32 *)((u8 *)D_800A34FC + 0x28) == 0x50005", "D_800A34FC->unk_28.word == 0x50005")
    c = "(s16)" if "s16cast" in OPT else ""
    b = sub1(b, "(s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] - 1)", c + "(D_800A34FC->unk_28.half[i] - 1)" if c else "D_800A34FC->unk_28.half[i] - 1")
    b = sub1(b, "(s16)(((s16 *)((u8 *)D_800A34FC + 0x28))[i] + 1)", c + "(D_800A34FC->unk_28.half[i] + 1)" if c else "D_800A34FC->unk_28.half[i] + 1")
    b = sub1(b, "((s16 *)((u8 *)D_800A34FC + 0x28))[i]", "D_800A34FC->unk_28.half[i]", 7)
    return b

def b6CFBC(b):
    return sub1(b, "*(s32 *)(D_800A34FC + 0x28) == 0x50005", "D_800A34FC->unk_28.word == 0x50005")

def b6D324(b):
    b = sub1(b, "    s16 *v1 = (s16 *)D_800A34FC;\n    v1[0x15] = 5;\n    v1[0x14] = 5;\n",
             "    Unk800A34FCRec *v1 = D_800A34FC;\n    v1->unk_28.half[1] = 5;\n    v1->unk_28.half[0] = 5;\n" if "v1" in OPT else
             "    D_800A34FC->unk_28.half[1] = 5;\n    D_800A34FC->unk_28.half[0] = 5;\n")
    return b

def b6E390(b):
    return sub1(b, "s0[1] = ((s32 *)D_800A34FC)[9];", "s0[1] = (s32)D_800A34FC->unk_24;")

BODIES = [("func_80068F70", b68F70), ("func_80069120", b69120), ("func_800693CC", xy),
          ("func_80069F80", xy), ("func_8006A1A0", xy), ("func_8006A880", xy), ("func_8006B120", xy),
          ("func_8006B578", xy), ("func_8006B92C", xy), ("func_8006BB68", xy), ("func_8006BD28", b6BD28),
          ("func_8006BEC4", b6BEC4), ("func_8006C21C", b6C21C), ("func_8006CBD4", b6CBD4),
          ("func_8006CCC8", b6CCC8), ("func_8006CFBC", b6CFBC), ("func_8006D324", b6D324),
          ("func_8006D3DC", xy), ("func_8006D5D4", xy), ("func_8006DD94", xy), ("func_8006DF68", xy),
          ("func_8006E390", b6E390)]
FUNCS = [f for f, _ in BODIES]

def src(s):
    s = sub1(s, "static s32 D_800A34FC;\n", "static Unk800A34FCRec *D_800A34FC;\n")
    for f, g in BODIES:
        s = fn(s, f, g)
    return s

BASE_REV = "71073622e"   # f03a as committed (equal to f03a.py's output)

def base():
    def show(p):
        return subprocess.run(["git", "show", "%s:%s" % (BASE_REV, p)], capture_output=True, check=True,
                              text=True, encoding="utf-8").stdout
    return show("src/main/51268.c"), show("include/game.h"), show("include/bb2.h"), show("src/main/64FD8.c")

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    s0, g0, h0, t = base()
    s = src(s0); g = game(g0); h = h0
    for n, x in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return s, g, h, t

if __name__ == "__main__":
    s, g, h, t = write()
    left = [l for l in s.split(NL) if W in l and "->" not in l and W + ";" not in l and W + " = " not in l]
    for l in left:
        print("LEFT", l.strip())
    if "measure" in sys.argv[1:]:
        fs = [a for a in sys.argv[1:] if a.startswith("func_")] or FUNCS
        A.Q.D2.D1.measure(s, g, h, t, fs)
    print("wrote f03b", sorted(OPT))
