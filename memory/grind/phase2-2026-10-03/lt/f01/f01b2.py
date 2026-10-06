#!/usr/bin/env python3
# F01 batch b2, on top of F01b1 (643264f38's 51268.c / game.h / bb2.h): D_800A3478
# becomes s16 * and D_800A347C s32 *, the pointers func_80060A68 aims at the D_800F116C block's unk18
# / unk20 (Unk1F800000Unk00); their readers drop the raw views. Unk1F800000Unk00.unk18 becomes s16[3]
# (its sources unk04 / unk10 and every consumer are s16), with D_800A346C and func_80061FAC's a0.
# usage: f01b2.py [measure [func...]] [opt=<name>,...]
#   writes tmp/p2/f01b2/{51268.c,game.h,bb2.h} (scratch only; never the tracked files);
#   with opt=..., tmp/p2/f01b2/opt/ (measurement variants).
import os, re, shutil, subprocess, sys
NL = chr(10)
B = "tmp/p2/f01b2/base/"
OUT = "tmp/p2/f01b2/"
OPT = set()
for a in sys.argv[1:]:
    if a.startswith("opt="):
        OPT |= set(a[4:].split(","))

for _p in ("src/main/51268.c", "include/game.h", "include/bb2.h"):
    _d = B + os.path.basename(_p)
    if not os.path.exists(_d):   # F01b1 as committed (643264f38), LF
        os.makedirs(B, exist_ok=True)
        open(_d, "wb").write(subprocess.run(["git", "show", "643264f38:" + _p], capture_output=True, check=True).stdout)

def rd(p):
    return open(p, encoding="utf-8").read()

def sub1(s, a, b, n=1):
    assert s.count(a) == n, (a[:90], s.count(a), n)
    return s.replace(a, b)

def span(s, f):
    m = re.search(r"\n[a-z0-9_]+ %s\([^;{]*\)\s*\{" % f, s)   # the definition, not a prototype
    i = m.start() + 1
    j = s.index("\n}\n", i) + 3
    return i, j

def fn(s, f, g):
    i, j = span(s, f)
    return s[:i] + g(s[i:j]) + s[j:]

# the 12 functions that copy *D_800A347C through `void *p` (func_80064E90 .. func_800652AC, without
# func_8006517C / func_800651F0, which hold it in an s32 * already)
VOIDP = ["func_80064E90", "func_80064ED8", "func_80064F20", "func_80064F68", "func_80064FB4",
         "func_80065000", "func_8006505C", "func_800650A4", "func_800650EC", "func_80065134",
         "func_80065264", "func_800652AC"]

FAKE_Q = (
    "    /* FAKE: the third word is read through a pointer to it: read as D_800A347C[2] (an address\n"
    "     * sum, so MEM_IN_STRUCT_P) it lets sched.c anti_dependence move the store to {flag} above\n"
    "     * that read and the unk4 store (score {score}, 19 insns either way). */\n")
SCORE_Q = {"D_800F10F4": "7", "D_800F10F8": "7"}
# `last`: measured needed in all 12 copiers (abl_b2.py: stored directly, score 4; 12 in the two
# with q, 8 for q and last together)
# (chk_last: the objdump of each ablation against the target)
LAST = ("    /* FAKE: named intermediate - with word 2 read inline, the D_800F0BA8 store (`sh`)\n"
        "       rises above the unk4 store and the word-2 read (score 4) */\n"
        "    s32 last;\n")
LAST_Q = ("    /* FAKE: named intermediate - with word 2 read inline, the {flag} store rises above the\n"
          "       unk0 / unk4 stores and the word-2 read, and the D_800F0BA8 store above the unk4 store\n"
          "       and the word-2 read (score 12; with q as well, 8) */\n"
          "    s32 last;\n")

def voidp(b):
    # D_800A347C read directly (the `void *p` holder goes: IDENTICAL without it)
    b = sub1(b, "    void *p = D_800A347C;\n", "")
    for k in range(3):
        b = sub1(b, "*(s32 *)((s32)p + %d)" % (4 * k), "D_800A347C[%d]" % k)
    m = re.search(r"\n    (D_800F10F[48]) = 1;", b)
    withq = bool(m and "] = 0x40;" in b)
    if withq and "noq" not in OPT:
        # func_80064F68 / func_80064FB4: the third word through a pointer (FAKE)
        flag = m.group(1)
        b = sub1(b, "    s32 last;\n", FAKE_Q.format(flag=flag, score=SCORE_Q[flag]) + "    s32 *q = &D_800A347C[2];\n" +
                 ("    s32 last;\n" if "nolabel" in OPT else LAST_Q.format(flag=flag)))
        b = sub1(b, "    last = D_800A347C[2];\n", "    last = *q;\n")
    elif "nolabel" not in OPT:
        b = sub1(b, "    s32 last;\n", LAST)
    return b

P517C = ("    s32 *p = D_800A347C; /* FAKE: one base for both walkers (each started from D_800A347C:\n"
         "                            score 12, 32 insns) */\n")
T517C = ("    /* FAKE: named intermediates - with a copy's word 2 stored directly, that copy's\n"
         "       D_800F0BA8 store (`sh`) rises above its unk4 store and the word-2 read (score 4\n"
         "       each, 8 both) */\n")

def p517c(b):
    # func_8006517C / func_800651F0: the second copy's third word was held in the s32 * p as an int
    # (`p = (s32 *)*bp; ... = (s32)p`); a fresh s32 local t2 is IDENTICAL
    b = sub1(b, "    s32 t;\n", "    s32 t;\n    s32 t2;\n")
    b = sub1(b, "    p = (s32 *)*bp;\n", "    t2 = *bp;\n")
    b = sub1(b, ".unk8 = (s32)p;", ".unk8 = t2;")
    if "walkers" not in OPT:
        # the p base and the ap / bp walkers go: D_800A347C[k] read directly is IDENTICAL
        # (abl_b2.py, *_walkers: score 0), as in the twelve siblings
        b = sub1(b, "    s32 *p = D_800A347C;\n    s32 *ap = p;\n    s32 *bp = p;\n", "")
        for w in ("ap", "bp"):
            b = b.replace("*%s++;" % w, "D_800A347C[0];", 1)
            b = b.replace("*%s++;" % w, "D_800A347C[1];", 1)
            b = sub1(b, "*%s;" % w, "D_800A347C[2];", 1)
        assert not re.search(r"\b[ab]p\b", b)
    elif "nolabel" not in OPT:
        b = sub1(b, "    s32 *p = D_800A347C;\n", P517C)
    if "nolabel" not in OPT:
        b = sub1(b, "    s32 t;\n    s32 t2;\n", T517C + "    s32 t;\n    s32 t2;\n")
    return b

def game(g):
    g = sub1(g, " * - unk1E: no access. */\ntypedef struct {\n    union {",
             " * - unk18 / unk20: the copies. D_800A346C / D_800A3470 point at the scratchpad block's,\n"
             " *   D_800A3478 / D_800A347C at the D_800F116C block's (func_80060A68). unk18 is s16 like\n"
             " *   its sources (unk04 / unk10) and every consumer (SVECTOR / SVec4i16 fields: func_80061FAC,\n"
             " *   func_8006288C, func_80063BD0, func_80067200).\n"
             " * - unk1E: no access. */\ntypedef struct {\n    union {")
    return sub1(g, "    u16 unk18[3];\n", "    s16 unk18[3];\n")

def bb2(h):
    return sub1(h, "extern void func_80061FAC(u16 *, s32 *, MATRIX *);", "extern void func_80061FAC(s16 *, s32 *, MATRIX *);")

BD0_OLD = " * through 10..19 and the slot is overwritten in rotation.\n *\n * Shape notes:\n *  - `for` loop with the found-arm INSIDE the loop and `break`: the loop's\n *    duplicated exit test (jump.c duplicate_loop_exit_test) plus the arm's\n *    skip label is what keeps the D_800A344C base copy in the preheader\n *    (cse.c cse_around_loop stops scanning at the first CODE_LABEL); a\n *    `goto found` arm after the loop coalesces the base.\n *  - `bits`/`mask` read before the test: the array read must be expanded\n *    before the `1 << i` so loop.c hoists the D_800A3454 address ahead of\n *    the constant 1 (their preheader order is the loop-body order).\n *  - A single trailing `return 1` that the else-arm falls into keeps\n *    `li v0,1` out of the else-arm block, which frees v0 there.\n */\nu8 func_80063BD0(s32 idx) {\n    s32 bits;\n    s32 mask;\n"
BD0_NEW = ' * through 10..19 and the slot is overwritten in rotation. */\nu8 func_80063BD0(s32 idx) {\n    s32 bits; /* FAKE: named intermediate - the D_800A3454[idx] word is read before `1 << i`, so\n                 loop.c hoists its address into the preheader ahead of the constant 1 (target:\n                 address in $t4, 1 in $t3); read in the test, or after mask: score 6, the two\n                 swap */\n    s32 mask;\n'

SETTRANS_OLD = """        /* SetTransMatrix reads only m->t (+0x14): hand it the address 0x14
           below tv so tv is loaded as the translation (base+0x10/0x12 hold
           w/h -- there is no whole MATRIX here). Spelled (MATRIX *)base, base
           stays live across the loop: +4 bytes. */
        SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));
"""
SETTRANS_NEW = """        /* FAKE: SetTransMatrix reads only m->t (+0x14): hand it the address 0x14
           below tv so tv is loaded as the translation (base+0x10/0x12 hold
           w/h -- there is no whole MATRIX here). Spelled (MATRIX *)base, base
           stays live across the loop: score 65, 502 insns for 501 (an 88-byte
           frame for 80; base in $s2 for the target's $s0). */
        SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));
"""

def src(s):
    s = sub1(s, "static s32 D_800A3478;\n", "static s16 *D_800A3478;\n")
    s = sub1(s, "static s32 D_800A347C;\n", "static s32 *D_800A347C;\n")
    s = sub1(s, "static u16 *D_800A346C;\n", "static s16 *D_800A346C;\n")
    s = sub1(s, "    D_800A3478 = (s32)D_800A3468->unk18;\n", "    D_800A3478 = D_800A3468->unk18;\n")
    s = sub1(s, "    D_800A347C = (s32)D_800A3468->unk20;\n", "    D_800A347C = D_800A3468->unk20;\n")
    # unk18 s16: func_80061FAC's a0, func_80060B70's dst_u16 (renamed dst16: the name stated u16),
    # func_800620B8's dst16
    s = sub1(s, "void func_80061FAC(u16 *a0, s32 *a1, MATRIX *a2) {", "void func_80061FAC(s16 *a0, s32 *a1, MATRIX *a2) {")
    s = fn(s, "func_80060B70", lambda b: re.sub(r"\bdst_u16\b", "dst16", sub1(b, "    u16 *dst_u16;\n", "    s16 *dst_u16;\n")))
    s = fn(s, "func_800620B8", lambda b: sub1(b, "    u16 *dst16;\n", "    s16 *dst16;\n"))
    # review fixes (rev-f01b2):
    # - func_800620B8's `pos` is the rotation angles (stored to unk04, copied to unk18, func_80061FAC's
    #   RotMatrix angle SVECTOR; 2B344 fills it with the negated camera rotation): neutral arg0
    s = sub1(s, "void func_800620B8(s16 *pos, s32 *trans) {", "void func_800620B8(s16 *arg0, s32 *trans) {")
    s = sub1(s, "    func_80060E38(pos, trans);\n", "    func_80060E38(arg0, trans);\n")
    s = sub1(s, "    rot = D_800A3474; /* matrix func_80061FAC builds from pos */\n",
             "    rot = D_800A3474; /* the matrix func_80061FAC builds from the angles at arg0 */\n")
    # - its tv - 0x14 SetTransMatrix idiom is load-bearing: label it (code text unchanged)
    s = sub1(s, SETTRANS_OLD, SETTRANS_NEW)
    # - func_80060B70's unlabelled last_arg block: the plain call is IDENTICAL
    if "lastarg" not in OPT:
        s = sub1(s, """    {
        MATRIX *last_arg = D_800A3474;
        dst_s32[2] = outer->unk08[2];
        func_80061FAC(dst16, dst_s32, last_arg);
    }
""", """    dst_s32[2] = outer->unk08[2];
    func_80061FAC(dst16, dst_s32, D_800A3474);
""")
    if "tvbase" in OPT:   # measurement: the idiom spelled (MATRIX *)base
        s = fn(s, "func_800620B8", lambda b: sub1(b, "        SetTransMatrix((MATRIX *)((u8 *)tv - 0x14));\n", "        SetTransMatrix((MATRIX *)base);\n"))
    # func_8006288C: rot is s16 * as at HEAD; only the cast goes
    s = sub1(s, "    pos = (s32 *)D_800A347C;\n    rot = (s16 *)D_800A3478;\n", "    pos = D_800A347C;\n    rot = D_800A3478;\n")
    s = sub1(s, "    src = (s32 *)D_800A347C;\n", "    src = D_800A347C;\n")
    s = sub1(s, "    s32 *p = (s32 *)D_800A347C;\n", "    s32 *p = D_800A347C;\n", 2)
    for f in VOIDP:
        s = fn(s, f, voidp)
    # func_80063BD0: its "Shape notes" (abl_b2.py): the found arm inside the loop with `break` and the
    # one trailing `return 1` are the plain forms (a goto arm scores 4, a return per arm 30), so their
    # notes go; `bits` is a FAKE named intermediate (6), noted at its declaration
    s = sub1(s, BD0_OLD, BD0_NEW)
    for f in ("func_8006517C", "func_800651F0"):
        s = fn(s, f, p517c)
    n = s.count("((s32 *)D_800A347C)[")
    assert n == 12, n
    s = s.replace("((s32 *)D_800A347C)[", "D_800A347C[")
    n = s.count("((u16 *)D_800A3478)[")
    assert n == 5, n
    s = s.replace("((u16 *)D_800A3478)[", "D_800A3478[")
    left = [l for l in s.split(NL) if re.search(r"D_800A347[8C]", l) and re.search(r"\(\s*(s32|u16|s16|void)\s*\**\s*\)\s*\(?\s*D_800A347[8C]", l)]
    assert not left, left
    return s

def write():
    d = OUT + ("opt/" if OPT else "")
    os.makedirs(d, exist_ok=True)
    s = src(rd(B + "51268.c")); g = game(rd(B + "game.h")); h = bb2(rd(B + "bb2.h"))
    for n, t in (("51268.c", s), ("game.h", g), ("bb2.h", h)):
        open(d + n, "w", encoding="utf-8", newline=NL).write(t)
    return s, g, h

def measure(s, g, h, fs=()):
    # scratch compile (tmp/p2/wk) against the base snapshot's object (HEAD 7b5212f70; F01b1 is IDENTICAL)
    shutil.rmtree("tmp/p2/wk", ignore_errors=True)
    os.makedirs("tmp/p2/wk/include"); os.makedirs("tmp/p2/wk/src/main")
    for n, t in (("include/game.h", g), ("include/bb2.h", h), ("src/main/51268.c", s)):
        open("tmp/p2/wk/" + n, "w", encoding="utf-8", newline=NL).write(t)
    r = subprocess.run(["wsl", "bash", "tmp/p2/item3/abl2.sh", "main/51268"] + list(fs), capture_output=True, text=True)
    print(r.stdout + r.stderr)
    t = subprocess.run(["wsl", "bash", "tmp/p2/item3/fcmp.sh", "main/51268"], capture_output=True, text=True)
    print(t.stdout + t.stderr)

if __name__ == "__main__":
    s, g, h = write()
    if "measure" in sys.argv[1:]:
        measure(s, g, h, [a for a in sys.argv[1:] if a.startswith("func_")])
    print("wrote b2", sorted(OPT))
