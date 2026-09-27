"""Per-TU declaration landing probe for func_8001CE60 (run ONLY while holding the landing lock).

Arrays in code6cac.c, single bytes in code6cac_b.c; the shared header no longer
declares the four score bytes. Symbol rows unchanged. D_800A377B handle retired.
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
h = sub1(h, "extern u8 D_800A377B;\n", "", "377B")
h = sub1(h, "extern u8 D_800A3898;\nextern u8 D_800A3899;\n", "", "3898")
h = sub1(h, "extern u8 D_800A38AA;\nextern u8 D_800A38AB;\n", "", "38AA")
wr("include/code6cac.h", h)

src = rd("src/code6cac.c")
cand = rd("memory/grind/func_8001CE60/candidate.c")
decl = ("/* P1/P2 round scores and tiebreakers, indexed by player (D_800A38B0) in\n"
        " * func_8001CE60. code6cac_b.c declares the same bytes as single u8s: see\n"
        " * memory/grind/func_8001CE60/evidence.md (s2, landing attempt 1). */\n"
        "extern u8 D_800A3898[2];\nextern u8 D_800A38AA[2];\n")
src = sub1(src, 'void func_8001CE60(void);\nINCLUDE_ASM("asm/funcs", func_8001CE60);\n', decl + cand, "splice")
wr("src/code6cac.c", src)

b = rd("src/code6cac_b.c")
b = sub1(b, "extern u8 D_800A377C;\n",
         "extern u8 D_800A377C;\n"
         "/* The score bytes as single u8s (code6cac.c declares them as [2] arrays):\n"
         " * see memory/grind/func_8001CE60/evidence.md (s2, landing attempt 1). */\n"
         "extern u8 D_800A3898;\nextern u8 D_800A3899;\nextern u8 D_800A38AA;\nextern u8 D_800A38AB;\n",
         "b decl")
wr("src/code6cac_b.c", b)
print("applied")
