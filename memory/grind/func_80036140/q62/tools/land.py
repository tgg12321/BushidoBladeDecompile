"""Apply the func_80036140 landing (Q62 COMMON model + CdState through 0x80101EA7) to a tree ROOT.
usage: python3 tmp/func_80036140/land.py <ROOT> <body.c>
Edits: include/code6cac.h, src/code6cac_b5.c, src/code6cac_b4_post.c, src/code6cac_b5_post.c,
named_syms.txt, undefined_syms_auto.txt. Writes LF."""
import re, sys
from pathlib import Path
sys.path.insert(0, '.')
from engine import inlineasm

root = Path(sys.argv[1]); body = Path(sys.argv[2]).read_text()


def rd(p):
    return (root / p).read_text()


def wr(p, t):
    (root / p).write_bytes(t.encode())


def rep(t, o, n):
    assert t.count(o) == 1, (t.count(o), o[:120])
    return t.replace(o, n)


# ---------------------------------------------------------------- include/code6cac.h
h = rd('include/code6cac.h')
h = rep(h, """ * declarations (owner Q62, 2026-09-30); the tentative definitions are in
 * src/code6cac_b4.c. */""", """ * declarations (owner Q62, 2026-09-30); the tentative definitions are in
 * the CD module's two -G8 units, src/code6cac_b4.c and src/code6cac_b5.c. */""")
h = rep(h, """ * the CdlATV blocks: result[0] is read gp-relative, result[4] with lui/%lo
 * (the owner Q62 global COMMON model; no C tentative definition yet). */""",
        """ * the CdlATV blocks: result[0] is read gp-relative, result[4] with lui/%lo
 * (the owner Q62 global COMMON model; the tentative definition is in
 * src/code6cac_b5.c). */""")
h = rep(h, """/* The replay-camera / CD-read words at 0x80101E60..0x80101E99: the tail of
 * CdState D_80101E58 below, where the evidence that they are one object with
 * its head is set out.  Member widths follow the original accesses;
 * 0x80101E91..93 is the compiler's alignment padding.  0x80101E9A..9B lies
 * inside the object (its size is a multiple of 4) and holds a halfword that
 * only func_80036140's asm accesses, through the D_80101E9A alias row; it has
 * no member (q2-review condition under owner ruling Q43) until that function
 * lands in C.
""", """/* The replay-camera / CD-read words at 0x80101E60..0x80101EA7: the tail of
 * CdState D_80101E58 below, where the evidence that they are one object with
 * its head is set out.  Member widths follow the original accesses;
 * 0x80101E91..93 is the compiler's alignment padding.  unk3A (0x80101E9A)
 * lies inside the proven span's object (its size is a multiple of 4) and is
 * func_80036140's halfword (sh 80036548, lhu 80036778 / sh 80036788).
 *
 * unk3C..unk44 (0x80101E9C..0x80101EA7) are in the object by compiler
 * necessity (aggregate-merge prong (a), (a1)/(a2)): func_80036140 is compiled
 * -G8, and there its read-modify-writes of 0x80101E9C and 0x80101EA4 keep the
 * address in a register (la; lX 0(r); sX 0(r)) only for a variable larger
 * than 8 bytes -- a smaller one is small data, and cse folds any pointer back
 * to the symbol.  Separate variables score 18, block-local pointers 18,
 * function-scope pointers 43, the object ending at 0x80101E9F or 0x80101EA3
 * (0x80101EA4 separate) 8, a separate 12-byte record at 0x80101E9C 8 (and
 * cdrom_ReadyCallback 12); this object 0.  unk3E and expected_pos lie inside
 * that span but func_80036140 never touches them: they are typed by their
 * other users' original accesses (owner rulings 2026-09-26 Q13/Q14): unk3E by
 * game_FrameLoop / cdrom_StartRead (u16, the lhu at 80036F9C); expected_pos by
 * cdrom_ReadyCallback / func_80036940 (s32: no access reveals its signedness,
 * and s32 / u32 build byte-identical).  Measurements and dumps:
 * memory/grind/func_80036140/evidence.md.
""")
h = rep(h, """    s16 unk38; /* 0x80101E98 */
} ReplayCamRec;""", """    s16 unk38; /* 0x80101E98 */
    s16 unk3A; /* 0x80101E9A */
    s16 unk3C; /* 0x80101E9C */
    u16 unk3E; /* 0x80101E9E */
    s32 expected_pos; /* 0x80101EA0 */
    s32 unk44; /* 0x80101EA4 */
} ReplayCamRec;""")
h = rep(h, """/* The CD module's state block, ONE object of 0x44 bytes at 0x80101E58.  Owner
 * ruling Q43 (2026-09-30, docs/grind/owner-rulings-2026-09-26.md) bounds it to
 * the span proven by the original binary, 0x80101E58..0x80101E99 (the size
 * rounds it up to 0x80101E9B; 0x80101E9A..9B: see ReplayCamRec above), by
 * three links:""", """/* The CD module's state block, ONE object of 0x50 bytes at 0x80101E58.  Owner
 * ruling Q43 (2026-09-30, docs/grind/owner-rulings-2026-09-26.md) bounds it to
 * the span proven: 0x80101E58..0x80101E99 by the original binary's addressing,
 * through 0x80101EA7 by func_80036140's compiler necessity (ReplayCamRec
 * above).  The first span is proven by three links:""")
h = rep(h, """ * 0x80101E9C, 0x80101E9E, 0x80101EA0 and 0x80101EA4 are declared separately
 * because no proof places them in this object (owner ruling Q43); C uses only
 * the two declared below. */""", """ */""")
h = rep(h, """    ReplayCamRec rec; /* 0x80101E60 .. 0x80101E99 */""", """    ReplayCamRec rec; /* 0x80101E60 .. 0x80101EA7 */""")
h = rep(h, "extern u16 D_80101E9E;\nextern s32 g_cdread_expected_pos;\n", "")
wr('include/code6cac.h', h)

# ---------------------------------------------------------------- src/code6cac_b5.c
b5 = rd('src/code6cac_b5.c')
b5 = rep(b5, """/* The CD module's two state-machine steppers, func_80036140 and func_80036940,
 * moved out of code6cac_b4_post.c together into their own translation unit
 * compiled -G8 (Makefile GP_FILES; owner ruling 2026-09-26, Q10): both read
 * g_cd_result straight off $gp, which the original compiler emits only at -G8.
 * func_80036140 is INCLUDE_ASM again (2026-09-30): its C matched only through the
 * per-function maspsx COMMON gate, which the owner ruled a cheat. */
#define INCLUDE_ASM_USE_MACRO_INC 1
#include "common.h"
#include "include_asm.h"
""", """/* The CD module's two state-machine steppers, func_80036140 and func_80036940,
 * moved out of code6cac_b4_post.c together into their own translation unit
 * compiled -G8 (Makefile GP_FILES; owner ruling 2026-09-26, Q10): both read
 * g_cd_result (and func_80036140 g_cd_atv, D_800A36B8, D_800A3840, D_800A3854)
 * straight off $gp, which the original compiler emits only at -G8.
 * g_cd_atv, D_800A36B8 and g_cd_result are declared here as in
 * code6cac_b4.c, the way their bytes show the original did: file-scope
 * tentative definitions (no initializer; their original bytes are zero).
 * Sony's assembler gave such a COMMON variable gp at its base only, never at
 * an offset (func_80036140 reads byte 0 of each gp-relative and the others
 * with lui/%lo); maspsx models that for every file (owner ruling Q62,
 * 2026-09-30, global COMMON model), not per function. */
#include "common.h"
""")
b5 = re.sub(r"/\* func_80036140's jump table.*?\};\n", "", b5, flags=re.S)
b5 = rep(b5, "extern s32 CdReady(s32, u8 *);\n",
         "extern s32 CdReady(s32, u8 *);\nCdlATV g_cd_atv;\nCdlATV D_800A36B8;\nu8 g_cd_result[8];\n")
b5 = inlineasm.substitute_body(b5, 'func_80036140', body)
b5 = rep(b5, 'g_cdread_expected_pos', 'D_80101E58.rec.expected_pos')
assert 'INCLUDE_ASM' not in b5
wr('src/code6cac_b5.c', b5)

# ---------------------------------------------------------------- consumers
b4p = rd('src/code6cac_b4_post.c')
assert b4p.count('g_cdread_expected_pos') == 3
wr('src/code6cac_b4_post.c', b4p.replace('g_cdread_expected_pos', 'D_80101E58.rec.expected_pos'))
b5p = rd('src/code6cac_b5_post.c')
assert b5p.count('D_80101E9E') == 2
wr('src/code6cac_b5_post.c', b5p.replace('D_80101E9E', 'D_80101E58.rec.unk3E'))

# ---------------------------------------------------------------- symbol files
RETIRE = ['g_cd_atv_plus_0x1', 'g_cd_atv_plus_0x2', 'g_cd_atv_plus_0x3', 'D_800A36B9', 'D_800A36BA',
          'D_800A36BB', 'D_80101E68', 'D_80101E62', 'D_80101E64', 'D_80101E6A', 'D_80101E74', 'D_80101E88',
          'D_80101E8C', 'D_80101E90', 'D_80101E94', 'D_80101E9A', 'D_80101E9C', 'D_80101E9E', 'D_80101EA4',
          'g_cd_loc', 'g_cd_result_plus_0x3', 'g_cd_result_plus_0x4', 'g_cd_result_plus_0x5',
          'g_cdread_expected_pos']
pat = re.compile(r'^(%s)\s*=' % '|'.join(RETIRE))
for f in ('named_syms.txt', 'undefined_syms_auto.txt'):
    lines = rd(f).split('\n')
    keep = [l for l in lines if not pat.match(l)]
    print(f, 'removed', len(lines) - len(keep))
    t = '\n'.join(keep)
    if f == 'named_syms.txt':
        t = rep(t, """/* 0x80101E58..0x80101E99 is ONE object, CdState D_80101E58 (include/code6cac.h, */
/* owner ruling Q43, 2026-09-30); the per-word rows in that span are retired. */""",
                """/* 0x80101E58..0x80101EA7 is ONE object, CdState D_80101E58 (include/code6cac.h, */
/* owner ruling Q43, 2026-09-30); the per-word rows in that span are retired. */""")
    wr(f, t)
print('landed into', root)
