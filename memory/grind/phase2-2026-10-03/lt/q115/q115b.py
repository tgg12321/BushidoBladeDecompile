#!/usr/bin/env python3
# Q115 batch b: typed island operands in 51268 func_80067D14 and 9F9C func_800203B4 (with the i203 set).
# func_80067D14: the gte_stlvnl output at work area +0x24 is the translation of a MATRIX at +0x10 (the
# SetTransMatrix island reads it from there), and the gte_stsxy3 outputs are D_800A34B8's three words.
# func_800203B4 takes the Unk80101EC8Record its six 17AFC callers hand it and stores by member; its
# gte_stlvnl output is &rec->unk_354.
# usage: q115b.py   writes tmp/p2/q115/outb/ (scratch); BASE_REV env (default 6151b1808, ":" = index);
#        OPT env: comma list; `name` adds a variant, `-name` drops a default.
import os, subprocess
NL = chr(10)
OUT = "tmp/p2/q115/outb/"
BASE_REV = os.environ.get("BASE_REV", "6151b1808")
DEFAULT = set()
OPT = set(DEFAULT)
for x in os.environ.get("OPT", "").split(","):
    if x.startswith("-"):
        OPT.discard(x[1:])
    elif x:
        OPT.add(x)

def show(p):
    spec = (":%s" % p) if BASE_REV == ":" else ("%s:%s" % (BASE_REV, p))
    return subprocess.run(["git", "show", spec], capture_output=True, check=True, text=True,
                          encoding="utf-8").stdout

def rep(t, pairs):
    for a, b in pairs:
        assert t.count(a) == 1, (a[:70], t.count(a))
        t = t.replace(a, b)
    return t

def game_h(t):
    return rep(t, [
        ("    u8 unk08[0x1C];                /* no access */\n    s32 unk24[3];                  /* func_80067D14: gte_stlvnl out */\n",
         "    u8 unk08[8];                   /* no access */\n"
         "    MATRIX unk10;                  /* func_80067D14: only its translation (+0x24) is used: the gte_stlvnl\n"
         "                                      output, which the SetTransMatrix island loads */\n"),
    ])

def bb2_h(t):
    return rep(t, [("extern void func_800203B4(u8 *, s32, s16 *);",
                    "extern void func_800203B4(Unk80101EC8Record *, s32, s16 *);")])

def s51268(t):
    pairs = [
        ("    p_out = outer->unk24;\n", "    p_out = outer->unk10.t;\n"),
        ('''            :: "r"((s32 *)D_800A34B8), "r"((s32 *)D_800A34B8 + 1),
               "r"((s32 *)D_800A34B8 + 2) : "memory");''',
         '''            :: "r"(&D_800A34B8[0]), "r"(&D_800A34B8[1]),
               "r"(&D_800A34B8[2]) : "memory");'''),
    ]
    if "pmat" in OPT:
        pairs[0] = ("    p_out = outer->unk24;\n", "    p_out = &outer->unk10;\n")
        pairs += [("    s32 *p_out;\n", "    MATRIX *p_out;\n"),
                  ('            :: "r"((u8 *)p_out - 0x14) : "$12", "$13", "$14");',
                   '            :: "r"(p_out) : "$12", "$13", "$14");')]
        t = rep(t, pairs)
        i = t.index("void func_80067D14(s32 arg0, s32 arg1) {")
        j = t.index("\n}\n", i)
        b = t[i:j]
        n1 = b.count('"r"(p_out)')
        b = b.replace(':: "r"(p_out) : "memory");', ':: "r"(p_out->t) : "memory");')
        for k in range(3):
            b = b.replace("p_out[%d]" % k, "p_out->t[%d]" % k)
        assert "p_out[" not in b
        return t[:i] + b + t[j:]
    if "mat_outer" in OPT:
        pairs.append(('            :: "r"((u8 *)p_out - 0x14) : "$12", "$13", "$14");',
                      '            :: "r"(&outer->unk10) : "$12", "$13", "$14");'))
    return rep(t, pairs)

def s9F9C(t):
    return rep(t, [
        (""" * materialize-then-copy anchor; see inline_asm_canonical.txt). Starts the arg0+0x350
 * frame counter, records bone index D_8008D59C[arg1].bone at arg0+0x352, and rotates
 * the vector arg2 by that bone's matrix of game_GetPlayerData(arg0+4) into arg0+0x354.""",
         """ * materialize-then-copy anchor; see inline_asm_canonical.txt). Starts the rec->unk_350
 * frame counter, records bone index D_8008D59C[arg1].bone at rec->unk_352, and rotates
 * the vector arg2 by that bone's matrix of game_GetPlayerData(rec->index) into rec->unk_354."""),
        (" *  - `arg0 += 0x354;` mirrors the SDK call shape (re-association is byte-neutral). */",
         " *  - the gte_stlvnl operand is &rec->unk_354. */"),
        ("void func_800203B4(u8 *arg0, s32 arg1, s16 *arg2) {", "void func_800203B4(Unk80101EC8Record *rec, s32 arg1, s16 *arg2) {"),
        ("    *(s16 *)(arg0 + 0x350) = 1;\n    *(s16 *)(arg0 + 0x352) = D_8008D59C[arg1].bone;\n"
         "    src = ((MATRIX **)game_GetPlayerData(*(s16 *)(arg0 + 4)))[*(s16 *)(arg0 + 0x352)];\n",
         "    rec->unk_350 = 1;\n    rec->unk_352 = D_8008D59C[arg1].bone;\n"
         "    src = ((MATRIX **)game_GetPlayerData(rec->index))[rec->unk_352];\n"),
        ("    arg0 += 0x354;\n    /* PsyQ libgte inline macro gte_stlvnl(r)", "    /* PsyQ libgte inline macro gte_stlvnl(r)"),
        ('        :: "r"(arg0) : "$12");\n}', '        :: "r"(&rec->unk_354) : "$12");\n}'),
    ])

def s17AFC(t):
    assert t.count("func_800203B4((u8 *)ch, limb, vec);") == 6
    t = t.replace("func_800203B4((u8 *)ch, limb, vec);", "func_800203B4(ch, limb, vec);")
    # func_80027AD8's carried labels get their measured scores (B4 sweep of the moved body)
    t = rep(t, [("     * its live length); Ruling 12. */\n    Tbl8008E194 *tbl;",
                 "     * its live length); Ruling 12. (rec used directly: score 78) */\n    Tbl8008E194 *tbl;")])
    t = rep(t, [("            sign = (u32)diff >> 31;\n",
                 "            /* FAKE: diff's sign bit taken ahead of the same / unk_B4 stores and the func_80032854 call; read\n"
                 "             * at its use in the table index, ch and limb swap $s0 / $s1 across the function: score 102 */\n"
                 "            sign = (u32)diff >> 31;\n")])
    old = "         * raises tbl's reg_n_refs before jump2 cross-jump merges it. */"
    old2 = "             * raises tbl's reg_n_refs before jump2 cross-jump merges it. */"
    assert t.count(old) == 3 and t.count(old2) == 1, (t.count(old), t.count(old2))
    t = t.replace(old2, "             * raises tbl's reg_n_refs before jump2 cross-jump merges it. (cases 0, 1-3 goto the case 4-5 copy: score 31) */")
    t = t.replace(old, "         * raises tbl's reg_n_refs before jump2 cross-jump merges it. (cases 0, 1-3 goto the case 4-5 copy: score 31) */")
    return t

def write():
    os.makedirs(OUT, exist_ok=True)
    out = {
        "51268.c": s51268(show("src/main/51268.c")),
        "9F9C.c": s9F9C(show("src/main/9F9C.c")),
        "17AFC.c": s17AFC(show("src/main/17AFC.c")),
        "game.h": game_h(show("include/game.h")),
        "bb2.h": bb2_h(show("include/bb2.h")),
    }
    for n, x in out.items():
        open(OUT + n, "w", encoding="utf-8", newline=NL).write(x)
    return out

if __name__ == "__main__":
    write()
    print("wrote q115b", sorted(OPT))
