#!/usr/bin/env python3
# F02 batch 5 (on HEAD b3af6a180): func_8002AB08 reads Unk1F8002B8Rec typed. unk00's point array
# widens from LeafPos[2] to LeafPos[6] (one layout: AB08 uses [0..5], A458 / 31B24 use [0..1]).
# AB08 passes `(u8 *)scr` to func_8002CD58's `u8 *obj` (batch-2 boundary-conversion ruling).
# usage: f02b5.py [measure]
import os, re, shutil, subprocess, sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b2 as B
NL = chr(10)
sub1, rd, span = B.sub1, B.rd, B.span

XYZ = "xyz"

def pt(off):
    assert off % 0xC == 0 and off < 0x48, hex(off)
    return "scr->unk00.unk00[%d]" % (off // 0xC)

def word(off):
    if off < 0x48:
        return "scr->unk00.unk00[%d].%s" % (off // 0xC, XYZ[(off % 0xC) // 4])
    if 0x78 <= off < 0x84:
        return "scr->unk78.%s" % XYZ[(off - 0x78) // 4]
    if 0x84 <= off < 0x90:
        return "scr->unk84.%s" % XYZ[(off - 0x84) // 4]
    if 0xC8 <= off < 0xD4:
        return "scr->unkC8.%s" % XYZ[(off - 0xC8) // 4]
    return {0xB4: "scr->unkB4", 0xC4: "scr->unkC4"}[off]

def body(b):
    n0 = b.count("scr")
    b = sub1(b, "    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;\n")
    # triangle pointers: *(u8 **)(scr + 0x60/64/68) = scr [+ off];
    def tri(m):
        k = (int(m.group(1), 16) - 0x60) // 4
        off = int(m.group(2), 16) if m.group(2) else 0
        return "scr->unk60[%d] = &%s;" % (k, pt(off))
    b = re.sub(r"\*\(u8 \*\*\)\(scr \+ (0x6[048])\) = scr(?: \+ (0x[0-9A-F]+))?;", tri, b)
    b = sub1(b, "*(LeafPos *)(scr + 0x84) = **(LeafPos **)(scr + 0x60);", "scr->unk84 = *scr->unk60[0];")
    b = sub1(b, "*(LeafPos *)(scr + 0x78) = *(LeafPos *)(scr + 0x84);", "scr->unk78 = scr->unk84;")
    b = re.sub(r"\(\*\(s32 \*\*\)\(scr \+ 0x60 \+ j \* 4\)\)\[([012])\]",
               lambda m: "scr->unk60[j]->%s" % XYZ[int(m.group(1))], b)
    b = re.sub(r"\*\(LeafPos \*\)\(scr \+ (0x[0-9A-F]+)\)", lambda m: pt(int(m.group(1), 16)), b)
    b = re.sub(r"\*\(s32 \*\)\(scr \+ (0x[0-9A-F]+)\)", lambda m: word(int(m.group(1), 16)), b)
    b = sub1(b, "func_8002CA8C(self, func_8002CD58(scr), deep_on);",
             "func_8002CA8C(self, func_8002CD58((u8 *)scr), deep_on);")
    left = re.findall(r".*scr \+.*|.*\(u8 \*\*\).*", b)
    assert not left, left
    return b

UNION_OLD = """/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch. unk00 is two points that unk60[0] / unk60[1] aim at
 * for func_8002E838 / func_8002EA24: func_8002A458's segment, base then tip (func_8002AB08 writes
 * them before each call), and func_80031B24's D_80106A78 object step, prev_pos then pos.
 * func_8002AB08 lays out more points after these two and still uses the bytes through its u8 *
 * pointer. func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument
 * block, its work area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of
 * Unk1F8002B8Union. raw sizes the union to 0x60. */
typedef union {
    u8 raw[0x60];
    LeafPos unk00[2];
} Unk1F8002B8Unk00;
"""
UNION_NEW = """/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch. unk00 is up to six points that unk60[0..2] aim at.
 * func_8002A458 and func_80031B24 use [0] / [1], for func_8002E838 / func_8002EA24:
 * func_8002A458's segment, base then tip (func_8002AB08 writes them before each call), and
 * func_80031B24's D_80106A78 object step, prev_pos then pos. func_8002AB08 uses all six: [0] / [1]
 * are two of the other character's scratchpad points (ScrPad.unk00 or unk48), [2] / [3] the same
 * two from its record (unk_210 or unk_234), [4] / [5] the midpoints of [0] / [2] and [1] / [3];
 * for each triangle it hands func_8002CD58 it aims unk60[0..2] at three of them.
 * func_80030D7C / func_800321E8 lay the bytes out differently (func_8005344C's argument block, its
 * work area from +0x38 through +0x123): Unk1F8002B8_8005344C, the other member of
 * Unk1F8002B8Union. raw sizes the union to 0x60. */
typedef union {
    u8 raw[0x60];
    LeafPos unk00[6];
} Unk1F8002B8Unk00;
"""

def game(g):
    g = sub1(g, UNION_OLD, UNION_NEW)
    a = g.index(" * them against the triangle (0,0) / unkA8 / unkB8. unk00 is scratch (Unk1F8002B8Unk00):\n")
    b = g.index(" */\ntypedef struct {\n    Unk1F8002B8Unk00 unk00;")
    old = g[a:b]
    assert "func_8002AB08 still uses it through its u8 *" in old, old
    new = (" * them against the triangle (0,0) / unkA8 / unkB8. unk00 is scratch (Unk1F8002B8Unk00):\n"
           " * func_8002A458 / func_80031B24 / func_8002AB08 use its points.")
    return g[:a] + new + g[b:]

def build():
    src = rd("src/main/17AFC.c")
    g = rd("include/game.h")
    i, j = span(src, "func_8002AB08")
    src = src[:i] + body(src[i:j]) + src[j:]
    return src, game(g)

def wk(src, g):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)

def measure(src, g, fs=("func_8002AB08",)):
    wk(src, g)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/17AFC"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/17AFC"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    src, g = build()
    open("tmp/p2/lt/f02/v/17AFC.b5.c", "w", encoding="utf-8", newline=NL).write(src)
    open("tmp/p2/lt/f02/v/game.h.b5", "w", encoding="utf-8", newline=NL).write(g)
    if "measure" in sys.argv[1:]:
        measure(src, g)
    print("wrote b5")
