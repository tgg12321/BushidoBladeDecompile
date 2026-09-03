/* func_80033550 — BYTES PROVEN (s16b, 2026-09-03, solver modality).
 * STATUS: INTEGRATION HANDOFF — the C is complete and measured, but it cannot
 * be landed by a grind session because the sanctioned spelling requires edits
 * to include/code6cac.h, undefined_syms_auto.txt and named_syms.txt, all of
 * which are outside a grind session's allowed surface.
 *
 * MEASURED THIS SESSION, with the three surface edits below applied:
 *   `sandbox func_80033550 --disable all` = score 0, target_insns 34,
 *   build_insns 34, rules_dropped 0
 *   `verify-oracle` = build_sha1 62efab4f73f992798c43e8c730aa43baa10bb4fa
 *   == oracle, build_matches true — measured TWICE: once with only the header
 *   merged, and again with the two per-word symbols also deleted from
 *   undefined_syms_auto.txt and named_syms.txt (prong (c) completeness).
 *   The surface diff is banked verbatim at
 *   tmp/grind/func_80033550/s16/integration_surfaces.patch.
 *
 * THE FINDING (inherited from the discarded s16a session, re-measured here).
 * Fifteen sessions hunted a byte-free register occupant to grow
 * hard_reg_conflicts[72] so find_reg's pass-0 scan (global.c:996-1001) would
 * skip $a1/$a2 and seat the arg0 pointer in $a3. The premise was wrong, not
 * the arithmetic: the tail is not three scalar load/store pairs, it is a
 * 12-byte AGGREGATE COPY. GCC 2.7.2's MIPS block-move expansion emits all
 * three loads before all three stores and keeps the source address register
 * live across the whole pattern, so pseudo 72 stops dying at the third load:
 *   scalar-triple tail : conflicts 2 3 4 29     -> pass0 first free 5 = $a1
 *   struct-copy   tail : conflicts 2 3 4 5 6 29 -> pass0 first free 7 = $a3
 * The conflict set arrives for free, at exactly 34 instructions, with no
 * FAKE construct, no dead local, no wrap, no preference route.
 *
 * WHY THIS SPELLING AND NOT THE PREVIOUS ONE. The discarded session spelled
 * the aggregate as a per-use byte-pointer pun,
 *   *(Word3 *) (((u8 *) (&D_80107850)) + i * 12) = *(Word3 *) arg0;
 * which the layer-1 cheat-reviewer FAILed (and the driver has since BANNED):
 * it is the "per-word splat symbol -> aggregate merge" family
 * (.claude/rules/no-new-park-categories.md:238) spelled so as to dodge that
 * family's prongs (c) (merge must be complete) and (d) (canonical declaration
 * in the shared header, never a per-use pointer pun). This form does the merge
 * properly instead: one canonical aggregate declaration in include/code6cac.h,
 * per-word symbols deleted everywhere, ordinary `D_80107850[i] = *arg0;` at
 * the use site with no magic stride constant.
 *
 * PRONG (a) EVIDENCE (independent of, and predating, any byte-chasing):
 *  - named_syms.txt:1556 (committed naming census)
 *      g_leaf_position_table = 0x80107850;
 *      /* 12-byte stride per leaf, 6 entries = 72-byte position array *_/
 *    plus named_syms.txt:2562-2563 which record 0x80107854/0x80107858
 *    explicitly as "+4 from" / "+8 from" that table.
 *  - Base-register stride indexing in the ORIGINAL BINARY, in a different
 *    function: asm/funcs/func_800335D8.s loads &D_80107850 into $s2
 *    (0x800335EC/F0) and walks the table with `addiu $s2, $s2, 0xC`
 *    (0x80033704) in lockstep with the D_800A3918 flag array walked at
 *    stride 1, bounded by D_800A391E = base+6 (six entries).
 *  - This function itself computes i*12 (`sll 1; addu; sll 2`, 0x80033594-9C)
 *    as the index into D_80107850.
 *
 * PRONG (e): full `verify-oracle` SHA1 == oracle with the complete merge
 * applied; the only other consumer of the symbol (func_800335D8) is still
 * INCLUDE_ASM and is unaffected; no C consumer of D_80107854/D_80107858 exists
 * (grep, this session).
 *
 * OPERATOR / NEXT-SESSION STEPS (all three surface edits, then the body):
 *   1. include/code6cac.h:478-480 — replace
 *        extern s32 D_80107850;
 *        extern s32 D_80107854;
 *        extern s32 D_80107858;
 *      with the typedef + array declaration below.
 *   2. undefined_syms_auto.txt:995-996 — delete the D_80107854 / D_80107858
 *      lines.
 *   3. named_syms.txt:2562-2563 — delete the g_leaf_position_table_plus_4 /
 *      _plus_8 rows.
 *   4. src/code6cac_b.c:2873 — replace INCLUDE_ASM with the body below.
 *   (The exact diff for 1-3 is tmp/grind/func_80033550/s16/integration_surfaces.patch.)
 */

/* ---- include/code6cac.h, replacing the three per-word externs ---- */
/* 12-byte per-leaf record table (named_syms.txt: g_leaf_position_table,
   "12-byte stride per leaf, 6 entries = 72-byte position array"). */
typedef struct {
    s32 x;
    s32 y;
    s32 z;
} LeafPos;

extern LeafPos D_80107850[6];

/* ---- src/code6cac_b.c:2873 ---- */
void func_80033550(LeafPos *arg0) {
    s32 i;

    for (i = 0; i < 6; i++) {
        if ((*(&D_800A3918 + i)) == 0) {
            break;
        }
    }
    if (i == 6) {
        return;
    }
    *(&D_800A3918 + i) = 1;
    D_80107850[i] = *arg0;
}
