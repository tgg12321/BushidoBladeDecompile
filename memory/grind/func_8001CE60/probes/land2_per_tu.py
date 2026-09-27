"""Per-file declaration landing for func_8001CE60 (run ONLY while holding the landing lock).

Owner ruling 2026-09-26 Q21: the score bytes are declared per file -- [2] arrays in
src/code6cac.c, single u8s in src/code6cac_b.c -- and no longer in the shared header.
Symbol rows unchanged.
usage: python3 tmp/func_8001CE60/land2.py
"""


def rd(p):
    with open(p, encoding="utf-8", newline="") as f:
        return f.read()


def wr(p, s):
    with open(p, "w", encoding="utf-8", newline="\n") as f:
        f.write(s)


def sub1(s, old, new, what):
    n = s.count(old)
    assert n == 1, (what, n)
    return s.replace(old, new)


h = rd("include/code6cac.h")
h = sub1(h, "extern u8 D_800A3898;\nextern u8 D_800A3899;\n", "", "3898")
h = sub1(h, "extern u8 D_800A38AA;\nextern u8 D_800A38AB;\n", "", "38AA")
wr("include/code6cac.h", h)

src = rd("src/code6cac.c")
cand = rd("memory/grind/func_8001CE60/candidate.c")
decl = ("/* P1/P2 round scores and tiebreakers, declared in this file as [2] arrays\n"
        " * because func_8001CE60 indexes them by player (D_800A38B0). src/code6cac_b.c\n"
        " * declares the same bytes as single u8s (D_800A3898, D_800A3899, D_800A38AA,\n"
        " * D_800A38AB) because func_800340A0 compiles to the shipped code only from\n"
        " * single-byte declarations (arrays: 5/88, the same under cc1psx).\n"
        " * Owner ruling 2026-09-26 Q21 (per-file declarations); evidence:\n"
        " * memory/grind/func_8001CE60/evidence.md, probes/calib_800340A0/. */\n"
        "extern u8 D_800A3898[2];\nextern u8 D_800A38AA[2];\n")
src = sub1(src, 'void func_8001CE60(void);\nINCLUDE_ASM("asm/funcs", func_8001CE60);\n', decl + cand, "splice")
wr("src/code6cac.c", src)

b = rd("src/code6cac_b.c")
b = sub1(b, "extern u8 D_800A377C;\n",
         "extern u8 D_800A377C;\n"
         "/* P1/P2 round scores and tiebreakers, declared in this file as single u8s\n"
         " * because func_800340A0 compiles to the shipped code only from single-byte\n"
         " * declarations (a [2] array puts the address in a register: 5/88, the same\n"
         " * under cc1psx). src/code6cac.c declares the same bytes as D_800A3898[2] /\n"
         " * D_800A38AA[2] because func_8001CE60 indexes them by player.\n"
         " * Owner ruling 2026-09-26 Q21 (per-file declarations); evidence:\n"
         " * memory/grind/func_8001CE60/evidence.md, probes/calib_800340A0/. */\n"
         "extern u8 D_800A3898;\nextern u8 D_800A3899;\nextern u8 D_800A38AA;\nextern u8 D_800A38AB;\n",
         "b decl")
wr("src/code6cac_b.c", b)
print("applied")
