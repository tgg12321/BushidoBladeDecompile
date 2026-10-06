#!/usr/bin/env python3
# F20 batch a, on top of F13 (f13.py over f03b): the settings record at 0x8009BD24 becomes one object,
# Unk8009BD24Block (game.h), declared as D_8009BD24. The split symbols D_8009BD38 (its +0x14 word),
# D_8009BD3B..3D (+0x17..), D_8009BD41..43 (+0x1D..) and D_8009BD44 (+0x20 word) leave C and
# undefined_syms_auto.txt; their by-name readers in 3AB48 / 51268 / 64FD8 read members.
# Unk8009BD38Flags (the +0x14 word alone) is folded into the block.
# usage: f20a.py [measure] [opt=<name>,...]   writes tmp/p2/f20a/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.join(HERE, "..", "f03"))
_argv = sys.argv
sys.argv = sys.argv[:1]
import f13 as F13
sys.argv = _argv
OUT = "tmp/p2/f20a/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = F13.sub1
fn = F13.fn
HEAD = "110ccc84e"

def show(p):
    return subprocess.run(["git", "show", "%s:%s" % (HEAD, p)], capture_output=True, check=True,
                          text=True, encoding="utf-8").stdout

# ---------------------------------------------------------------- game.h
OLD_FLAGS = """/* 0x8009BD38: the match-settings flag word, bit fields named by bit offset.
   Its C readers extract it by field: unk0 (`& 0xF`), unk10 (the round count
   - 3; also picks the results-screen layout), unk12 (`== 2` tests), unk14
   (1 bit), unk15 (one bit per player); func_80077894 stores unk0. Byte 3 is
   not named here (main/64FD8.c reads it as D_8009BD3B). */
typedef struct {
    u32 unk0 : 4;
    u32 unk4 : 6;
    u32 unk10 : 2;
    u32 unk12 : 2;
    u32 unk14 : 1;
    u32 unk15 : 2;
    u32 unk17 : 1;
    u32 unk18 : 6;
} Unk8009BD38Flags;
"""

NEW_BLOCK = """/* One of the three 4-byte records at Unk8009BD24Block.unk21 (func_8003C714 and func_80035280 store
   them; func_8006D808 reads them). */
typedef struct {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
} Unk8009BD45Rec;

/* 0x8009BD24..0x8009BD57: the settings record func_80077D00 returns and 64FD8 hands to
   func_80068F70 / func_8006E534 / func_800770B8, whose callees reach every field off that one
   base (51268 D_800A3524, 5ED34 D_800A3568, SelWork.f00). Bit fields are named by bit offset.
   - unk14_*: the word at +0x14. unk14_0 (`& 0xF`; func_80077894 stores it), unk14_4 (6 bits),
     unk14_10 (the round count - 3; also picks the results-screen layout), unk14_12 (`== 2`
     tests), unk14_14, unk14_15 (one bit per player), unk14_17, unk14_18 (3 bits); no code
     reads bits 21-23.
   - unk17 / unk1A / unk1D: three byte triples. func_8006CCC8 rebuilds a player's nibble of unk17
     from unk1A or unk1D; 64FD8 copies unk17 and unk1D into each other.
   - unk20_*: the word at +0x20, bits 0-3; its upper bytes start the unk21 records. */
typedef struct {
    Unk8009BD24Record unk00[2][5];
    u32 unk14_0 : 4;
    u32 unk14_4 : 6;
    u32 unk14_10 : 2;
    u32 unk14_12 : 2;
    u32 unk14_14 : 1;
    u32 unk14_15 : 2;
    u32 unk14_17 : 1;
    u32 unk14_18 : 3;
    u32 unk14_21 : 3;
    u8 unk17[3];
    u8 unk1A[3];
    u8 unk1D[3];
    u32 unk20_0 : 1;
    u32 unk20_1 : 1;
    u32 unk20_2 : 1;
    u32 unk20_3 : 1;
    u32 unk20_4 : 4;
    Unk8009BD45Rec unk21[3];
    u8 unk2D[3];
    u8 unk30;
} Unk8009BD24Block;
"""

OLD_RECORD = """/* 0x8009BD24: two players x five rounds of 2-byte records; byte 0 is the
   character the round was fought with (func_8005E54C reads it at
   j * 10 + i * 2 and picks UesrWorkDef / D_8009B58C by it; func_80060414 reads
   player 0 round 0). 0x14 bytes, ending at the flag word below. */
typedef struct {
    u8 chr;
    u8 unk1;
} Unk8009BD24Record;

"""

def game(g):
    # the record, the clock record and the block go ahead of SelWork (whose f00 points at the block)
    g = sub1(g, OLD_RECORD + OLD_FLAGS + NL, "")
    rec = OLD_RECORD.replace("player 0 round 0). 0x14 bytes, ending at the flag word below. */",
                             "player 0 round 0). Unk8009BD24Block.unk00. */")
    anchor = "/* The select-screen work area D_800A36A0 points at"
    return sub1(g, anchor, rec + NEW_BLOCK + "\n" + anchor)

def bb2(h):
    h = sub1(h, " * caches [n][1] in D_800A35E0, n = D_8009BD38.unk0 (base + n*2,",
             " * caches [n][1] in D_800A35E0, n = D_8009BD24.unk14_0 (base + n*2,")
    return sub1(h, "extern Unk8009BD24Record D_8009BD24[2][5];\n", "extern Unk8009BD24Block D_8009BD24;\n")

# ---------------------------------------------------------------- sources
def s3AB48(t):
    t = sub1(t, "\n\nextern Unk8009BD38Flags D_8009BD38;\n", "\n")
    for f in ("unk0", "unk10", "unk12", "unk14", "unk15"):
        t = re.sub(r"\bD_8009BD38\.%s\b" % f, "D_8009BD24.unk14_%s" % f[3:], t)
    t = re.sub(r"\bD_8009BD24\[", "D_8009BD24.unk00[", t)
    assert "D_8009BD38" not in t
    return t

def s51268(s):
    s = sub1(s, "extern Unk8009BD38Flags D_8009BD38;\n", "")
    s = sub1(s, "D_8009BD38.unk0", "D_8009BD24.unk14_0")
    s = sub1(s, "    extern s32 D_8009BD44[];\n", "", 5)
    s = sub1(s, "D_8009BD44[0] & 1", "D_8009BD24.unk20_0", 5)
    s = sub1(s, "(D_8009BD44[0] & 8)", "D_8009BD24.unk20_3", 7)
    assert "D_8009BD44" not in s and "D_8009BD38" not in s
    return s

def s64FD8(t):
    t = sub1(t, "extern Unk8009BD38Flags D_8009BD38;\n", "")
    t = re.sub(r"\bD_8009BD38\.unk0\b", "D_8009BD24.unk14_0", t)
    t = sub1(t, """extern u8 D_8009BD3B;
extern u8 D_8009BD3C;
extern u8 D_8009BD3D;
extern u8 D_8009BD41;
extern u8 D_8009BD42;
extern u8 D_8009BD43;
""", "")
    for k, (a, i) in {"3B": ("17", 0), "3C": ("17", 1), "3D": ("17", 2),
                      "41": ("1D", 0), "42": ("1D", 1), "43": ("1D", 2)}.items():
        t = re.sub(r"\bD_8009BD%s\b" % k, "D_8009BD24.unk%s[%d]" % (a, i), t)
    t = sub1(t, "&D_8009BD24[0][0].chr", "&D_8009BD24.unk00[0][0].chr")
    t = sub1(t, "    return (s32 *)D_8009BD24;\n", "    return (s32 *)&D_8009BD24;\n")
    assert not re.search(r"D_8009BD(38|3[B-D]|4[1-4])\b", t)
    return t

def syms(u):
    for k in ("41", "43", "3D", "3B", "42", "38", "3C", "44"):
        u = sub1(u, "D_8009BD%s = 0x8009BD%s;\n" % (k, k), "")
    return u

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    s, g, h, t = F13.write()
    a = show("src/main/3AB48.c"); u = show("undefined_syms_auto.txt")
    out = {"51268.c": s51268(s), "game.h": game(g), "bb2.h": bb2(h), "64FD8.c": s64FD8(t),
           "3AB48.c": s3AB48(a), "undefined_syms_auto.txt": syms(u)}
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    out = write()
    print("wrote f20a", sorted(OPT))
