/* KILLED s4 frontier #2 (2026-07-19): move v1 into guard-block scope so
 * it exists ONLY inside the loop; a0<<4 inlined into s4's init at file scope.
 *
 * Hypothesis (frontier): CSE across scopes may still fold both a0<<4 uses
 * to one insn while removing v1's file-scope pseudo, avoiding the CSE-move
 * copy that killed cse-fold-anon-shift.
 *
 * Result: sandbox --disable all -> score=35, build_insns=84 (target 83).
 * CSE did NOT fold across scopes: the loop's inner `v1 = a0<<4` becomes a
 * fresh pseudo inside the do-while block and disrupts RA severely (35-point
 * register-choice cascade). The extra insn also confirms non-fold.
 *
 * VERDICT KILLED: cross-scope CSE fold hypothesis is FALSE for GCC 2.7.2
 * in this shape. Scoping v1 to the loop is strictly worse than the
 * cse-fold-anon-shift near-hit (which at least kept 2, +1 insn).
 */
void func_80045294(s32 a0, s32 a1) {
    s32 sum = 0;
    s32 s4 = *(s32 *)((u8 *)&D_800EED14 + (a0 << 4));
    s32 i = a0;
    s32 count = D_800A33AC;
    s32 s5 = s4 + a1;

    if (i < count) {
        s32 v1 = a0 << 4;
        do {
            s32 val = *(s32 *)((u8 *)&D_800EED18 + v1);
            v1 += 0x10;
            i += 1;
            sum += val;
        } while (i < count);
    }
    /* ... rest unchanged ... */
}
