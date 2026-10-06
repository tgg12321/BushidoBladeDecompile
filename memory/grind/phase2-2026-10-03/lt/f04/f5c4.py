#!/usr/bin/env python3
# FZZ 5ED34 (on f07.py): 5ED34's work block D_800A35C4, the 0x14 bytes func_8006E534 keeps at the start
# of func_8006E49C's returned space (D_800A356C += 0x14), as 51268's D_800A34FC: Unk800A35C4Rec.
# usage: f5c4.py [opt=<name>,...]   writes tmp/p2/f5c4/ (scratch only)
import os, re, subprocess, sys
NL = chr(10)
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
_argv = sys.argv
sys.argv = sys.argv[:1]
import f07 as P
sys.argv = _argv
OUT = "tmp/p2/f5c4/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))
sub1 = P.sub1
fn = P.fn
OFS = "s16" if "ofs_s16" in OPT else "u16"

TYPE = """/* 5ED34's work block: the 0x14 bytes func_8006E534 keeps at the start of func_8006E49C's returned
 * space (D_800A35C4; the arena cursor D_800A356C moves past it), as 51268's Unk800A34FCRec.
 * - unk_00 / unk_04: one s16 per player each (counted down from 0x1E by the draw handlers).
 * - unk_08: the frame counter func_8006EACC advances; unk_0C: its buffer counter (bit 0 picks the
 *   half of the Unk8006E49CRec pair).
 * - unk_10: the draw offset the handlers hand to SetDrawOffset. */
typedef struct {
    s16 unk_00[2];
    s16 unk_04[2];
    s32 unk_08;
    s32 unk_0C;
    %s unk_10[2];
} Unk800A35C4Rec;

""" % OFS

def game(g):
    anchor = "/* 51268's work block: the 0x34 bytes func_80068F70 keeps"
    return sub1(g, anchor, TYPE + anchor)

R = "D_800A35C4"

def s5ED34(e):
    e = sub1(e, "static void * D_800A35C4;\n", "static Unk800A35C4Rec *D_800A35C4;\n")
    e = sub1(e, "    D_800A35C4 = (void *)D_800A356C;\n", "    D_800A35C4 = (Unk800A35C4Rec *)D_800A356C;\n")
    reps = [
        (r"\*\(s32 \*\)\(\(s32\)\(?D_800A35C4\)? \+ 8\)", "D_800A35C4->unk_08"),
        (r"\*\(s32 \*\)\(\(s32\)D_800A35C4 \+ 0xC\)", "D_800A35C4->unk_0C"),
        (r"\(\(s32 \*\)D_800A35C4\)\[2\]", "D_800A35C4->unk_08"),
        (r"\(\(s32 \*\)D_800A35C4\)\[3\]", "D_800A35C4->unk_0C"),
        (r"\(\(s16 \*\)D_800A35C4\)\[i \+ 2\]", "D_800A35C4->unk_04[i]"),
        (r"\(\(s16 \*\)D_800A35C4 \+ 2\)\[", "D_800A35C4->unk_04["),
        (r"\(\(s16 \*\)D_800A35C4\)\[8 \+ i\]", "D_800A35C4->unk_10[i]"),
        (r"\(\(s16 \*\)D_800A35C4\)\[2\]", "D_800A35C4->unk_04[0]"),
        (r"\(\(s16 \*\)D_800A35C4\)\[3\]", "D_800A35C4->unk_04[1]"),
        (r"\(\(s16 \*\)D_800A35C4\)\[(i|0|1)\]", r"D_800A35C4->unk_00[\1]"),
        (r"\(\(u16 \*\)D_800A35C4\)\[8\]", "D_800A35C4->unk_10[0]"),
        (r"\(\(u16 \*\)D_800A35C4\)\[9\]", "D_800A35C4->unk_10[1]"),
        (r"\(u16 \*\)D_800A35C4 \+ 8", "D_800A35C4->unk_10"),
        (r"\*\(u16 \*\)\(D_800A35C4 \+ 0x10\)", "D_800A35C4->unk_10[0]"),
        (r"\*\(u16 \*\)\(D_800A35C4 \+ 0x12\)", "D_800A35C4->unk_10[1]"),
        (r"D_800A35C4 \+ 0x10\)", "D_800A35C4->unk_10)"),
    ]
    for a, b in reps:
        e = re.sub(a, b, e)
    def g(b):
        b = sub1(b, "        u8 *offset;\n", "        Unk800A35C4Rec *offset;\n")
        b = sub1(b, "        *(s16 *)(offset + 0x10) = offset_x;\n", "        offset->unk_10[0] = offset_x;\n")
        return b
    e = fn(e, "func_8006F528", g)
    e = sub1(e, " * D_800A35C4 + 8 (incremented in func_8006EACC), each", " * D_800A35C4->unk_08 (incremented in func_8006EACC), each")
    if OFS == "s16":
        e = e.replace("SetDrawOffset(arg0->unk_04.unk_1C, D_800A35C4->unk_10)", "SetDrawOffset(arg0->unk_04.unk_1C, (u16 *)D_800A35C4->unk_10)")
    left = [l.strip() for l in e.split(NL) if R in l and "D_800A35C4->" not in l and "static Unk800A35C4Rec" not in l]
    for l in left:
        print("LEFT", l)
    return e

def write():
    d = OUT + ("opt_%s/" % "_".join(sorted(OPT)) if OPT else "")
    os.makedirs(d, exist_ok=True)
    out = dict(P.write())
    out["5ED34.c"] = s5ED34(out["5ED34.c"])
    out["game.h"] = game(out["game.h"])
    for n, x in out.items():
        open(d + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote f5c4", sorted(OPT))
