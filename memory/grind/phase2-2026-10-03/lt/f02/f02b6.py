#!/usr/bin/env python3
# F02 batch 6 (on HEAD da99a0242): the last raw Unk1F8002B8Rec users outside the asm-operand bodies.
# func_800290B8, func_80029454 (+ box_overlap, func_8002DE20's forward prototype), func_8002C22C,
# func_8002C61C's two func_800283D0 arguments. func_8002DAD0 keeps `u8 *obj` (raw asm operands):
# `(u8 *)scr` is the batch-2 boundary conversion.
# usage: f02b6.py [opt=halve] [measure]
import os, re, shutil, subprocess, sys
sys.path.insert(0, "tmp/p2/lt/f02")
import f02b2 as B
NL = chr(10)
sub1, rd, span, edit = B.sub1, B.rd, B.span, B.edit
OPT = set(a[4:] for a in sys.argv[1:] if a.startswith("opt="))
XYZ = "xyz"
VEC = {0x78: "unk78", 0x84: "unk84", 0x90: "unk90", 0x9C: "unk9C", 0xA8: "unkA8", 0xB8: "unkB8",
       0x100: "unk100[0]", 0x13C: "unk13C"}

def member(off):
    for base, name in VEC.items():
        if base <= off < base + 0xC:
            return "scr->%s.%s" % (name, XYZ[(off - base) // 4])
    raise KeyError(hex(off))

def rawsub(b):
    b = re.sub(r"\*\*\(LeafPos \*\*\)\(scr \+ 0x(60|6C)\)",
               lambda m: "*scr->unk%s[0]" % m.group(1), b)
    b = re.sub(r"\(\(LeafPos \*\*\)\(scr \+ 0x(60|6C)\)\)\[k\]", lambda m: "scr->unk%s[k]" % m.group(1), b)
    def ptr(m):
        off = int(m.group(1), 16)
        base = 0x60 if off < 0x6C else 0x6C
        return "scr->unk%X[%d]" % (base, (off - base) // 4)
    b = re.sub(r"\*\(LeafPos \*\*\)\(scr \+ (0x[0-9A-F]+)\)", ptr, b)
    b = re.sub(r"\*\(LeafPos \*\)\(scr \+ (0x[0-9A-F]+)\)", lambda m: "scr->" + VEC[int(m.group(1), 16)], b)
    b = re.sub(r"\*\(s32 \*\)\(scr \+ (0x[0-9A-F]+)\)", lambda m: member(int(m.group(1), 16)), b)
    b = b.replace("(s32 *)(scr + 0x100)", "&scr->unk100[0].x")
    return b

def f290b8(s):
    i, j = span(s, "func_800290B8")
    b = s[i:j]
    b = sub1(b, "    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;\n")
    b = rawsub(b)
    assert "scr +" not in b, re.findall(r".*scr \+.*", b)
    s = s[:i] + b + s[j:]
    s = sub1(s, " * Builds the x/z bounds and the top y of grid `idx` in the scratchpad record\n"
                " * at 0x1F8002B8 (min at +0x78, max at +0x84), then walks the PosRec list for\n"
                " * an unused entry inside those bounds and tests it (at +0x100) against the\n",
             " * Builds the x/z bounds and the top y of grid `idx` in the scratchpad record\n"
             " * at 0x1F8002B8 (min in unk78, max in unk84), then walks the PosRec list for\n"
             " * an unused entry inside those bounds and tests it (at unk100[0]) against the\n")
    s = sub1(s, " * missing while `flag` is set and idx is not 0, stores at +0x104 the average\n"
                " * of +0x7C (the y of point idx * 4, which the scan never updates) and the top\n"
                " * y, and returns 1. Returns 0 when the list runs out.\n",
             " * missing while `flag` is set and idx is not 0, stores in unk100[0].y the\n"
             " * average of unk78.y (the starting grid's point 0; the scan never updates it)\n"
             " * and the top y (unk84.y), and returns 1. Returns 0 when the list runs out.\n")
    return s

BOX_OLD = """/* 1 when the box at scr+0x78 (min) / +0x84 (max) and the box at scr+0x90 (min)
 * / +0x9C (max) overlap on all three axes. */
static inline s32 box_overlap(u8 *scr) {
    return *(s32 *)(scr + 0x78) <= *(s32 *)(scr + 0x9C) && *(s32 *)(scr + 0x84) >= *(s32 *)(scr + 0x90)
        && *(s32 *)(scr + 0x7C) <= *(s32 *)(scr + 0xA0) && *(s32 *)(scr + 0x88) >= *(s32 *)(scr + 0x94)
        && *(s32 *)(scr + 0x80) <= *(s32 *)(scr + 0xA4) && *(s32 *)(scr + 0x8C) >= *(s32 *)(scr + 0x98);
}

/* Both are defined further down this file. func_8002DE20 has no prototype here: it takes
 * the Unk1F8002B8Rec its callers hold as a u8 * `scr`. */
extern s32 func_8002DAD0(u8 *obj);
extern s32 func_8002DE20();
"""
BOX_NEW = """/* 1 when the box unk78 (min) / unk84 (max) and the box unk90 (min) / unk9C (max)
 * overlap on all three axes. */
static inline s32 box_overlap(Unk1F8002B8Rec *scr) {
    return scr->unk78.x <= scr->unk9C.x && scr->unk84.x >= scr->unk90.x
        && scr->unk78.y <= scr->unk9C.y && scr->unk84.y >= scr->unk90.y
        && scr->unk78.z <= scr->unk9C.z && scr->unk84.z >= scr->unk90.z;
}

/* Both are defined further down this file. */
extern s32 func_8002DAD0(u8 *obj);
extern s32 func_8002DE20(Unk1F8002B8Rec *obj, LeafPos *p0, LeafPos *p1, LeafPos *p2);
"""

def f29454(s):
    s = sub1(s, BOX_OLD, BOX_NEW)
    i, j = span(s, "func_80029454")
    b = s[i:j]
    b = sub1(b, "    u8 *scr = (u8 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;\n")
    b = b.replace("func_8002DE20(scr, *(s32 **)(scr + 0x6C), *(s32 **)(scr + 0x70), *(s32 **)(scr + 0x74))",
                  "func_8002DE20(scr, scr->unk6C[0], scr->unk6C[1], scr->unk6C[2])")
    b = b.replace("func_8002DAD0(scr)", "func_8002DAD0((u8 *)scr)")
    b = rawsub(b)
    if "halve" in OPT:
        b = sub1(b, "    s32 *p;\n", "    LeafPos *p;\n")
        b = sub1(b, "            p = (s32 *)((u8 *)ws + (i * 0x60 + j * 0x18));\n"
                    "            p[0] >>= 1;\n            p[1] >>= 1;\n            p[2] >>= 1;\n"
                    "            p[3] >>= 1;\n            p[4] >>= 1;\n            p[5] >>= 1;\n",
                 "            p = &ws[i * 8 + j * 2];\n"
                 "            p[0].x >>= 1;\n            p[0].y >>= 1;\n            p[0].z >>= 1;\n"
                 "            p[1].x >>= 1;\n            p[1].y >>= 1;\n            p[1].z >>= 1;\n")
    left = re.findall(r".*scr \+.*", b)
    assert not left, left
    s = s[:i] + b + s[j:]
    s = sub1(s, " * points 2,3,1 (t even) or 0,1,2 (t odd) of grid t >> 1, written as pointers\n"
                " * to scr+0x60.. (first triangle) or scr+0x6C.. (second).\n",
             " * points 2,3,1 (t even) or 0,1,2 (t odd) of grid t >> 1, written as pointers\n"
             " * to scr->unk60[] (first triangle) or scr->unk6C[] (second).\n")
    s = sub1(s, " * record-1 triangle (bounding boxes at scr+0x78 / scr+0x90, then\n",
             " * record-1 triangle (bounding boxes unk78 / unk84 and unk90 / unk9C, then\n")
    s = sub1(s, " * 1, effects 1, 0x26 and 0x2D play at scr+0x100, +0x286 becomes 0x19 (+0x8C\n",
             " * 1, effects 1, 0x26 and 0x2D play at scr->unk100[0], +0x286 becomes 0x19 (+0x8C\n")
    return s

C22C_HDR_OLD = """/* Accumulates a character's shadow vectors in PSX scratchpad RAM.  Everything
 * this function writes lives in the one record at 0x1F8002B8 that `scr` points
 * at (an s32 view; each index is the byte offset / 4): two 3-word vectors at
 * +0xA8 and +0xB8 (the +0xB4 and +0xC4 words are not touched, so both are
 * 16-byte-strided like a PsyQ VECTOR), and the 3-word result at +0x13C.
 * +0xA8 sums two of the scratchpad points SPAD holds per character (unk48[k]
 * when bit k of D_800A3824 is set, else unk00[k]); +0xB8 sums the matching
 * pair func_8002C61C copied into D_80101EC8[k] (unk_234 / unk_210).
"""
C22C_HDR_NEW = """/* Accumulates a character's shadow vectors in PSX scratchpad RAM.  Everything
 * this function writes lives in the one record at 0x1F8002B8 that `scr` points
 * at: two vectors, unkA8 and unkB8 (this function does not touch unkB4 /
 * unkC4), and the result, unk13C.
 * unkA8 sums two of the scratchpad points SPAD holds per character (unk48[k]
 * when bit k of D_800A3824 is set, else unk00[k]); unkB8 sums the matching
 * pair func_8002C61C copied into D_80101EC8[k] (unk_234 / unk_210).
"""

def f2c22c(s):
    s = sub1(s, C22C_HDR_OLD, C22C_HDR_NEW)
    i, j = span(s, "func_8002C22C")
    b = s[i:j]
    b = sub1(b, "    s32 *scr = (s32 *)0x1F8002B8;\n", "    Unk1F8002B8Rec *scr = &SPAD->unk2B8.rec;\n")
    b = re.sub(r"scr\[0x([0-9A-F]+)/4\]", lambda m: member(int(m.group(1), 16)), b)
    assert "scr[" not in b
    return s[:i] + b + s[j:]

C61C_FAKE_OLD = """     * $s1 / $s0 from the prologue (lui/addiu s1, addiu s0,s1,0x44C) and reads
     * unk_AD / unk_3C / unk_286 / unk_0C / unk_F4 / unk_28C at displacements off
     * them, while unk_6A and the unk_210 / unk_234 copy loops use the absolute
     * address.  Without the pointers every access is a symbol+offset constant
     * address, which GO_IF_LEGITIMATE_ADDRESS
     * (tools/gcc-2.7.2/config/mips/mips.h:2286) accepts as is, so no base
     * register exists. */
"""
# Review fixes (rev-f02b6, rev-f02b6b): the ablation keeps bases in $s0 / $s1, and unk_AD is
# absolute in the target too. The two (u16) casts on the u16 unk_6A are no-ops (IDENTICAL without).
C61C_FAKE_NEW = """     * $s1 / $s0 from the prologue (lui/addiu s1, addiu s0,s1,0x44C) and reads
     * unk_3C / unk_286 / unk_0C / unk_F4 / unk_28C at displacements off them,
     * while unk_AD, unk_6A and the unk_210 / unk_234 copy loops use the
     * absolute address.  Without the pointers those five fields' accesses
     * become absolute lui pairs (GO_IF_LEGITIMATE_ADDRESS, gcc-2.7.2 mips.h:2286)
     * and cse keeps &D_80101EC8[0].unk_6A in $s0 instead (later also
     * &D_80101EC8[0].unk_286 in $s1): 12 more instructions. */
"""

def f2c61c(s):
    return edit(s, "func_8002C61C", [("(s32 *)0x1F8003F4", "&SPAD->unk2B8.rec.unk13C.x", 2),
                                      (C61C_FAKE_OLD, C61C_FAKE_NEW),
                                      ("(u16)D_80101EC8[0].unk_6A == 5", "D_80101EC8[0].unk_6A == 5"),
                                      ("(u16)D_80101EC8[1].unk_6A == 5", "D_80101EC8[1].unk_6A == 5")])

def build():
    src = rd("src/main/17AFC.c")
    g = rd("include/game.h")
    for f in (f290b8, f29454, f2c22c, f2c61c):
        src = f(src)
    return src, g

FS = ["func_800290B8", "func_80029454", "func_8002C22C", "func_8002C61C"]

def wk(src, g):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/17AFC.c", "w", encoding="utf-8", newline=NL).write(src)

def measure(src, g, fs=FS):
    wk(src, g)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/17AFC"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/17AFC"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    src, g = build()
    tag = "b6" + ("h" if "halve" in OPT else "")
    open("tmp/p2/lt/f02/v/17AFC.%s.c" % tag, "w", encoding="utf-8", newline=NL).write(src)
    if "measure" in sys.argv[1:]:
        measure(src, g)
    print("wrote", tag)
