#!/usr/bin/env python3
"""Apply the func_800759D0 + func_80075F80 joint landing to copies (or, with --inplace, to the real files).
usage: build_scratch.py <outdir> [b759.c] [bf80.c]   |   build_scratch.py --inplace [b759.c] [bf80.c]
Writes <outdir>/{text1b_tu2.c,text1b_tu1e.c,game.h}. Bytes, LF."""
import sys

inplace = sys.argv[1] == "--inplace"
out = None if inplace else sys.argv[1]
b759 = sys.argv[2] if len(sys.argv) > 2 else "tmp/j759/b759.c"
bf80 = sys.argv[3] if len(sys.argv) > 3 else "tmp/j759/bf80.c"


def rd(p):
    return open(p, "rb").read().decode()


def rep(s, old, new, what, count=1):
    n = s.count(old)
    assert n == count, f"{what}: {n} != {count}"
    return s.replace(old, new)


# ---- game.h
g = rd("include/game.h")
old_tbl_doc_start = g.index("/* 0x8009BCF8: 40-byte table of 20 two-byte records")
old_tbl_doc_end = g.index("typedef struct {\n    u8 unk0;\n    u8 unk1;\n} Unk8009BCF8Record;")
g = g[:old_tbl_doc_start] + """/* 0x8009BCF8: 2 pages x 10 two-byte records (0x8009BCF8..0x8009BD1F; D_8009BD20
 * follows), one page per character-select page, 2 rows x 5 columns of cells.
 * Object model evidence from the original binary: func_80075F80 (0x800761B4-
 * 0x800761E4) and func_800759D0 (0x80075BC4-0x80075BF0) address it as
 * page * 20 + cell * 2 -- the cell (row * 5 + col) shifted left 1, the page times
 * 5 shifted left 2, added, then `lbu %lo(D_8009BCF8)($at)` -- two-level array
 * indexing of [page][cell]; func_800759D0's cell loop steps through the records
 * flat from the table base (`lui $s6,%hi(D_8009BCF8); addiu $s6,$s6,%lo(D_8009BCF8)`,
 * a 2-byte step), and func_80076D74 reads byte 1 (`lbu %lo(D_8009BCF9)($at)`)
 * through a flat shift-1 index. Data: the unk1 column is 0x00..0x09 (page 0) then
 * 0x0C..0x15 (page 1). Replaces the splat per-word scalars D_8009BCF8 / D_8009BCF9
 * (per-word splat symbol -> aggregate merge family, owner ruling 2026-08-17). */
""" + g[old_tbl_doc_end:]
g = rep(g, "extern Unk8009BCF8Record D_8009BCF8[20];", "extern Unk8009BCF8Record D_8009BCF8[2][10];", "tbl decl")

# ---- text1b_tu2.c
t = rd("src/text1b_tu2.c")
t = rep(t, 'INCLUDE_ASM("asm/funcs", func_800759D0);\n', rd(b759), "759D0")
t = rep(t, 'INCLUDE_ASM("asm/funcs", func_80075F80);\n', rd(bf80), "75F80")
t = rep(t, "                hdr->cells[i][j][0] = D_8009BCF8[SELWORK->f6A[i][j]].unk1;\n",
        "                /* flat character index into both pages, through row 0. */\n"
        "                /* SOTN: src/st/e_grave_keeper.h:534 @aa53500 */\n"
        "                hdr->cells[i][j][0] = D_8009BCF8[0][SELWORK->f6A[i][j]].unk1;\n", "76D74")
a = t.index("/* func_80076D74 - Judge-CLEARED body")
b = t.index(" * See evidence.md / hypotheses.md.\n */\n", a) + len(" * See evidence.md / hypotheses.md.\n */\n")
t = t[:a] + """/* func_80076D74: select-screen fade-out.  Raises SELWORK->f36 by 8 per frame
 * (capped at 0xFF) and, once it reaches 0xFF, fills the result record at
 * SELWORK->f00 (f65, f66, f67, f68 packed into bitfields; per player and pick,
 * the picked entry's D_8009BCF8 column 1 and its f7E value).  Every frame it
 * draws the full-screen TILE with f36 as its colour.  The record and
 * bitfield layout, the do-while(0) tail and their measurements:
 * 3e35ec719^:memory/grind/func_80076D74/hypotheses.md (ledger closed at
 * 3e35ec719). */
""" + t[b:]
t = rep(t, "lever-exhaustion: memory/grind/func_80076D74/hypotheses.md s1-s2 */",
        "lever-exhaustion: 3e35ec719^:memory/grind/func_80076D74/hypotheses.md s1-s2 */", "76D74 path")


if inplace:
    open("include/game.h", "wb").write(g.encode())
    open("src/text1b_tu2.c", "wb").write(t.encode())
else:
    import os
    os.makedirs(out, exist_ok=True)
    open(out + "/game.h", "wb").write(g.encode())
    open(out + "/text1b_tu2.c", "wb").write(t.encode())
print("ok")
