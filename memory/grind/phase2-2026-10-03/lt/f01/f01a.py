#!/usr/bin/env python3
# F01 batch a (on HEAD a8ffddc34): 51268's fixed scratchpad-field globals become typed pointers to the
# members of Unk1F800000Rec, 51268's view of the scratchpad (17AFC's is ScrPad). The cursors
# (D_800A3468 / 3478 / 347C / 3488 / 348C / 34E4 / 34E8), the command block at +0x00 (D_800A346C /
# 3470 point into it) and the work area at +0xB8 (D_800A34EC) are later batches.
# usage: f01a.py [measure]
import os, re, shutil, subprocess, sys
NL = chr(10)

def rd(p):
    return open(p, encoding="utf-8").read()

def sub1(s, a, b, n=1):
    assert s.count(a) == n, (a[:90], s.count(a), n)
    return s.replace(a, b)

# global -> (offset, field type, member expression for the seed)
G = {
    "D_800A3474": (0x30, "MATRIX", "&SPAD51268->unk30"),
    "D_800A3490": (0x58, "s32", "&SPAD51268->unk58"),
    "D_800A3494": (0x5C, "s32", "&SPAD51268->unk5C"),
    "D_800A3498": (0x60, "u16", "&SPAD51268->unk60"),
    "D_800A349C": (0x62, "u16", "&SPAD51268->unk62"),
    "D_800A34A0": (0x64, "u16", "&SPAD51268->unk64"),
    "D_800A34A4": (0x66, "u16", "&SPAD51268->unk66"),
    "D_800A34A8": (0x68, "s16", "&SPAD51268->unk68"),
    "D_800A34AC": (0x6A, "s16", "&SPAD51268->unk6A"),
    "D_800A34B0": (0x6C, "s32", "&SPAD51268->unk6C"),
    "D_800A34B4": (0x70, "s32", "&SPAD51268->unk70"),
    "D_800A34B8": (0x74, "s32", "SPAD51268->unk74"),
    "D_800A34BC": (0x80, "s16", "&SPAD51268->unk80"),
    "D_800A34C0": (0x82, "s16", "&SPAD51268->unk82"),
    "D_800A34C4": (0x84, "s32", "&SPAD51268->unk84"),
    "D_800A34C8": (0x88, "s32", "&SPAD51268->unk88"),
    "D_800A34CC": (0x8C, "s32", "&SPAD51268->unk8C"),
    "D_800A34D0": (0x90, "s32", "SPAD51268->unk90"),
    "D_800A34D4": (0x98, "u16", "&SPAD51268->unk98"),
    "D_800A34D8": (0x9A, "u16", "&SPAD51268->unk9A"),
    "D_800A34DC": (0x9C, "u16", "&SPAD51268->unk9C"),
    "D_800A34E0": (0x9E, "u16", "&SPAD51268->unk9E"),
    "D_800A3480": (0xA8, "s32", "&SPAD51268->unkA8"),
    "D_800A3484": (0xAC, "s32", "&SPAD51268->unkAC"),
}
OTHER_SIGN = {"u16": "s16", "s16": "u16"}

REC = """/* 51268's view of the scratchpad from 0x1F800000 (SPAD51268 in 51268.c). 17AFC's view of the same
 * memory, used at other times, is ScrPad. func_80060E38 points the file's D_800A34xx globals at these
 * members (51268.c); apart from its own two raw stores to unk00's +0x04 / +0x08, the code reaches
 * them only through those globals:
 * - unk00: the command block D_800A3468 points at until a function retargets it (func_80060A68's
 *   Ob layout; D_800A346C / D_800A3470 point at its +0x18 / +0x20), with +0x2C..+0x2F. Typed with
 *   D_800A3468 in a later batch.
 * - unk50 / unkB0: the initial targets of D_800A3488 / D_800A348C. func_800620B8 retargets them
 *   only in its switch cases 0-3 (unk4 & 7), so its u16 reads at 51268.c:989-991 go through these
 *   seeded values when no earlier record took one of those cases (typed with the globals later).
 * - unkA0: the initial targets of D_800A34E4 / D_800A34E8, which every user retargets before use.
 * - unkB8: the work area D_800A34EC points at, laid out differently by its users (later batch);
 *   it runs to the end of the scratchpad.
 * The other members are one global each (D_800A3474 .. D_800A34E0, D_800A3480 / D_800A3484). */
typedef struct {
    u8 unk00[0x30];
    MATRIX unk30;      /* D_800A3474 */
    u8 unk50[8];
    s32 unk58;         /* D_800A3490 */
    s32 unk5C;         /* D_800A3494 */
    u16 unk60;         /* D_800A3498 */
    u16 unk62;         /* D_800A349C */
    u16 unk64;         /* D_800A34A0 */
    u16 unk66;         /* D_800A34A4 */
    s16 unk68;         /* D_800A34A8 */
    s16 unk6A;         /* D_800A34AC */
    s32 unk6C;         /* D_800A34B0 */
    s32 unk70;         /* D_800A34B4 */
    s32 unk74[3];      /* D_800A34B8 */
    s16 unk80;         /* D_800A34BC */
    s16 unk82;         /* D_800A34C0 */
    s32 unk84;         /* D_800A34C4 */
    s32 unk88;         /* D_800A34C8 */
    s32 unk8C;         /* D_800A34CC */
    s32 unk90[2];      /* D_800A34D0 */
    u16 unk98;         /* D_800A34D4 */
    u16 unk9A;         /* D_800A34D8 */
    u16 unk9C;         /* D_800A34DC */
    u16 unk9E;         /* D_800A34E0 */
    u8 unkA0[8];
    s32 unkA8;         /* D_800A3480 */
    s32 unkAC;         /* D_800A3484 */
    u8 unkB0[8];
    u8 unkB8[0x400 - 0xB8];
} Unk1F800000Rec;

"""

def game(g):
    anchor = "/* Unk1F8002B8Rec.unk00, 0x60 bytes of scratch."
    i = g.index("typedef struct {\n    s16 x;\n    s16 z;\n} Cell_80052D00;")
    i = g.rindex("/*", 0, i)
    return g[:i] + REC + g[i:]

ASM = re.compile(r"__asm__\s*(?:__volatile__|volatile)?\s*\(.*?\);", re.S)

def src(s):
    # asm text and operands are never edited: park every __asm__ statement while substituting
    parked = []
    def park(m):
        parked.append(m.group(0))
        return "@@ASM%d@@" % (len(parked) - 1)
    s = ASM.sub(park, s)
    s = _src(s)
    s = re.sub(r"@@ASM(\d+)@@", lambda m: parked[int(m.group(1))], s)
    # review fixes (rev-f01a2): casts left on two rewritten lines -- `(s32 *)sv` contradicts
    # RotTransPers' SVECTOR * (sv is SVECTOR *); `(s32)*(s16 *)&D_800A3440` re-reads the s16 static
    # with its own type. Both measured IDENTICAL without.
    s = sub1(s, "RotTransPers((s32 *)sv, D_800A34B8, p, D_800A34CC);", "RotTransPers(sv, D_800A34B8, p, D_800A34CC);")
    s = sub1(s, "*D_800A3484 = (s32)*(s16 *)&D_800A3440;", "*D_800A3484 = D_800A3440;")
    # review fix (rev-f01a): func_8006295C's `end` FAKE comment, as at HEAD, misstated the ablation
    return sub1(s, """           quad; mechanism: global.c priority -- a fresh cursor local
           (nrefs 10 / livelen 15) outranks the zbuf[k] giv and takes s0,
           while prim's pseudo is already seated in s1 as in the target. */
""", """           quad (target: prim s1, the zbuf[k] giv s0, end s2). Linked through
           a fresh cursor local instead, prim loses the tail refs and drops
           to s2, the cursor takes s0 and the giv s1: score 37. */
""")

def _src(s):
    for name, (off, t, seed) in G.items():
        s = sub1(s, "static s32 %s;\n" % name, "static %s *%s;\n" % (t, name))
        s = sub1(s, "    %s = 0x1F8000%02X;\n" % (name, off) if off >= 0x10 else None, "    %s = %s;\n" % (name, seed))
        n = len(re.findall(r"\b%s\b" % name, s))
        s = re.sub(r"\*\(%s \*\)%s\b" % (t, name), "*" + name, s)
        s = re.sub(r"\(\(%s \*\)%s\)\[" % (t, name), name + "[", s)
        s = re.sub(r"\(%s \*\)%s\b" % (t, name), name, s)
        if t in OTHER_SIGN:
            # stores through the other signedness: the same sh
            s = re.sub(r"\*\(%s \*\)%s( = )" % (OTHER_SIGN[t], name), "*" + name + r"\1", s)
        assert len(re.findall(r"\b%s\b" % name, s)) == n
    s = sub1(s, "void func_80060E38(s16 *arg0, s32 *arg1) {\n",
             "#define SPAD51268 ((Unk1F800000Rec *)0x1F800000)\n"
             "void func_80060E38(s16 *arg0, s32 *arg1) {\n")
    return s

def build():
    return src(rd("tmp/p2/lt/f01/51268.head.c")), game(rd("tmp/p2/lt/f01/game.head.h"))

def wk(s, g):
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    shutil.copyfile("include/bb2.h", "tmp/p2/wk/include/bb2.h")
    open("tmp/p2/wk/include/game.h", "w", encoding="utf-8", newline=NL).write(g)
    open("tmp/p2/wk/src/main/51268.c", "w", encoding="utf-8", newline=NL).write(s)

def measure(s, g, fs=()):
    wk(s, g)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/51268"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

def libgte(h):
    # RotTransPers4's flag (PsyQ `long *flag`, the header's own comment): D_800A34CC is now s32 *.
    return sub1(h, "s32 *, s32 *, s32 *, s32 *, s32 *, s32);", "s32 *, s32 *, s32 *, s32 *, s32 *, s32 *);")

if __name__ == "__main__":
    open("tmp/p2/lt/f01/v/libgte.h.a", "w", encoding="utf-8", newline=NL).write(libgte(rd("tmp/p2/lt/f01/libgte.head.h")))
    s, g = build()
    os.makedirs("tmp/p2/lt/f01/v", exist_ok=True)
    open("tmp/p2/lt/f01/v/51268.a.c", "w", encoding="utf-8", newline=NL).write(s)
    open("tmp/p2/lt/f01/v/game.h.a", "w", encoding="utf-8", newline=NL).write(g)
    if "measure" in sys.argv[1:]:
        measure(s, g)
    print("wrote a")
