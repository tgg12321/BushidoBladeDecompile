/* KILLED s4 (2026-07-19): inner-block scope defer — v1 declared at top
 * uninit, v1 = a0<<4 assigned as a statement AFTER `s32 i = a0;` init,
 * then opening an inner `{` block for s4/s5 decls and the whole function
 * body. Hypothesis: putting v1 = a0<<4 at a later STATEMENT position
 * (post-i) would push its LUID above move16's; the inner block would keep
 * s4/s5 as inner-scope pseudos preserving RA priority for a0->s2.
 *
 * Result: sandbox --disable all -> score=11, build_insns=83.
 * OBJDUMP prologue (RA rotated exactly like H1 kill):
 *   move $21,$4          <-- a0 -> $21 (was $18)
 *   move $19,$5          <-- a1 -> $19 unchanged
 *   move $16,$21         <-- s0 = a0's new register ($21)
 *   sll  $3,$16,0x4      <-- sll operand CSE-propagated to $16
 *
 * VERDICT KILLED: score-11 = the exact same shape as
 * rejected/i-before-v1-init.c and rejected/decl-init-decouple.c. This
 * confirms (a third angle) that RA priority follows ASSIGNMENT LUID,
 * NOT statement position within a nested block scope. The inner `{...}`
 * did not create a new RA phase; global.c allocates all pseudos at once
 * across the outer function.
 *
 * COUPLING CONFIRMED (from three angles now):
 * - s1 H1: reorder init order i-before-v1 -> RA rotates.
 * - s3 decl-init-decouple: split decl from init -> RA rotates.
 * - s4 inner-block-defer (this): use nested block scope -> RA rotates.
 * RA priority is on the ASSIGNMENT LUID axis with no known lever to
 * decouple it from the sched2 LUID axis at C-source level.
 */
void func_80045294(s32 a0, s32 a1) {
    s32 v1;
    s32 sum = 0;
    s32 i = a0;
    s32 count = D_800A33AC;
    v1 = a0 << 4;
    {
        s32 s4 = *(s32 *)((u8 *)&D_800EED14 + v1);
        s32 s5 = s4 + a1;
        /* rest of body unchanged */
    }
}
