/* func_80052B7C -- STRUCTURAL LEVERS EXHAUSTED as of s3 (this session). No
 * pure-C landing form exists under the current rules; every prior frontier
 * item is now measured dead. Read hypotheses.md s2/s3/H4/H5 for the full
 * derivation; summary below.
 *
 * What's PROVEN (s2, re-verified s3, sandbox func_80052B7C --disable all
 * --diff, honest floor 16 against the INCLUDE_ASM-stub floor of 24):
 *   #include <psxsdk/inline_c.h>   (NOT inline_o.h -- see s2 finding 1a)
 *   void func_80052B7C(MATRIX *rot, SVECTOR *tr, VECTOR *v0, VECTOR *out) {
 *       gte_SetRotMatrix(rot);   // chunk 1 -- WRONG instruction order (see below)
 *       <chunk 2: no vendored macro exists for TR-from-halfwords -> CR5-7>
 *       gte_ldv0(v0);            // chunk 3 -- CONFIRMED byte-identical
 *       gte_rt();                // chunk 4 -- CONFIRMED byte-identical
 *       gte_stlvl(out);          // chunk 5 -- WRONG delay-slot placement (see below)
 *   }
 *
 * What's DEAD, and why each is now a closed question rather than an open
 * frontier item:
 *   - Chunk 1 (gte_SetRotMatrix): both vendored headers interleave 2-lw+2-ctc2
 *     then 3-lw+3-ctc2; target batches all 5 lw then all 5 ctc2. No finer
 *     vendored leaf or gtemac.h composite exists to compose the batched order
 *     (s3 H2: full leaf enumeration; this session's H5: full gtemac.h visual
 *     read -- no composite names or expands to gte_SetRotMatrix at all).
 *   - Chunk 2 (TR from halfwords into CR5-7): no vendored macro of any kind
 *     performs this op (s1).
 *   - Chunk 5 (gte_stlvl): target's last swc2 sits in the jr $ra delay slot;
 *     our build emits it before jr with an explicit nop. Confirmed via .dbr
 *     RTL dump (s3 H3): gte_stlvl's 3-swc2 body is one opaque asm_operands
 *     insn reorg.c's delay-slot filler cannot split or select into.
 *   - A written __asm__ island closing any of the three (chunks 1, 2 or 5)
 *     is RULED OUT: docs/grind/decisions.md 2026-10-15 20:38 func_80052B7C
 *     ruling, citing inline-asm-policy.md:55-57/70-72 -- "the macro can't
 *     reach the bytes" is not an exception anywhere in the rule. This
 *     session's H4 confirms the ruling matches the live rule text. The
 *     previously-banked written-island candidate is now in
 *     rejected/written-island-chunks-1-2-5.c; do not resubmit it or any
 *     respelling of the same construct.
 *
 * What's LEFT, and why it is NOT a grind-session action:
 *   - A new vendored macro / DMPSX word covering chunk 2 needs its own owner
 *     ruling plus an `engine:` commit (inline-asm-policy.md item 1) -- and
 *     would still leave chunks 1 and 5 unreached (their macros already exist
 *     and still don't reach the bytes).
 *   - The whole-function inline_asm_canonical.txt:340 grant (reopened for
 *     audit by aeea8ed44, owner 2026-10-08) would hold AS-IS if
 *     scan_hand_coded reached STRONG tier (precedent: the func_8004A76C
 *     2026-10-15 CANONICAL-ASM GRANT PATH entry in docs/grind/decisions.md).
 *     Re-checked fresh this session: `scan_hand_coded.py --single
 *     func_80052B7C` = LOW 0/12, no gate-class signal
 *     (tmp/grind/func_80052B7C/s3/scan.txt). Raising this needs a scanner
 *     signal-set change (engine/tools), not a C edit.
 *
 * No src/ edit is banked here: the honest floor (16, with the now-rejected
 * written island) cannot be re-landed, and no replacement reaches a lower
 * honest floor without the banned construct. src/main/3AB48.c keeps
 * INCLUDE_ASM("asm/funcs", func_80052B7C); per asm-until-matched rule 1.
 *
 * s4 (permuter, mandated modality): decomp-permuter's standard import path
 * (tools/decomp-permuter/import.py) cannot even build a scorable workspace
 * for this function -- its default assembler (plain mips-linux-gnu-as) has
 * no cop2 GTE opcodes and fails on asm/funcs/func_80052B7C.s:22's `mvmva`.
 * No campaign was launched (nothing to launch against). See
 * hypotheses.md s4/H6 for the full measurement and tmp/grind/func_80052B7C/
 * s4/ for the run log. Even a hand-built workspace (routing target.s through
 * the project's own maspsx/aspsx pipeline, untested) would have nothing to
 * mutate: the macro-call chassis has zero free C-level variables, and the
 * unresolved chunks' divergence sits inside each macro's own opaque
 * asm_operands RTL node (s3/H3, dump-confirmed), unreachable by statement
 * reordering and not reachable without the already class-killed written-
 * island construct (s3/H4).
 */

/* s6 (synthesis): best LEGAL form measured -- pure vendored-macro calls only,
 * no __asm__ in the body (satisfies the Judge constraint's first branch).
 * sandbox func_80052B7C --disable all = 19 (stub 24; the rejected written-island
 * form was 16). Chunk 2 lands via gte_ldtr (inline_c.h:371) with C-level lh loads;
 * (s32) casts are semantically required (TRX/TRY/TRZ are 32-bit signed -- without
 * them GCC emits lhu, a zero-extension, wrong for negative translations; score
 * masks it, both 19). Residual = chunks 1 and 5 only (macro-internal order /
 * delay slot), source-level per --diff.
 * Apply by replacing INCLUDE_ASM("asm/funcs", func_80052B7C); in src/main/3AB48.c.
 */
/* s7 (synthesis): body unchanged, still 19. cc1psx calibration: SN cc1psx emits a
 * byte-identical body from this C (tmp/grind/func_80052B7C/s7/cc1_vs_cc1psx.txt).
 * Chunk 5 class-closed: reorg.c:740-744 never fills a delay slot with an asm insn and
 * mips.md has no cop2 store pattern, so no C body puts swc2 $11 in the jr slot. Only the
 * whole-body hand-written grant branch can land this function.
 */
#include <psxsdk/inline_c.h>
void func_80052B7C(MATRIX *rot, SVECTOR *tr, SVECTOR *v0, VECTOR *out) {
    gte_SetRotMatrix(rot);
    gte_ldtr((s32)tr->vx, (s32)tr->vy, (s32)tr->vz);
    gte_ldv0(v0);
    gte_rt();
    gte_stlvl(out);
}
