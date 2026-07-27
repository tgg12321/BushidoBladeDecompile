/* s4 permuter find (base-random-classify, output-10-1, 44.7 s after seed):
 * score 10 = 2 insn diffs — REJECTED: SEMANTICS-DIVERGENT FALSE FLIP.
 *
 * The mutation folds the 0x7FFF mask into the store (`*(v1-1) = a4 & 0x7FFF`)
 * WITHOUT masking a4 itself, so the loop counter runs UNMASKED — the loop
 * executes 0x8000 extra iterations vs the original function. Not a valid
 * decomp of func_80044098 at any score.
 *
 * Why it scores 10 anyway (decoded, s4): dropping the counter's mask set
 * removes 2 counter refs; with dw0 double-weighting + the a6 split-init the
 * pointer outranks the counter and the flip constellation lands. The only
 * residual is the mask temp: `andi v0,a0,0x7fff / sw v0` vs target
 * `andi a0,a0 / sw a0`. Closing those 2 diffs REQUIRES the masked value to
 * BE the counter in $a0 (mask-into-counter), which restores the counter's
 * refs and reverts the flip — this basin cannot reach 0 from this topology.
 *
 * Component decomposition (s4 probes, workspace compile at offset 0,
 * stripped-dump diff lines; base = 26 lines = 13 insn diffs):
 *   pB3  a6 split-init alone (`a6 = v1; a6 = a6 - 1;`)        : 26 = INERT
 *   pB1  a6 split-init + dw0 wrapper (mask intact)            : 26 = INERT
 *   pB4  semantics-correct re-mask (`a4 = (a4 & 0x7FFF) - 1`) : 17 @ 25
 *        insns — the s1 GUARD-FOLD kill (pre-decrement value in a separate
 *        pseudo -> combine folds guard to beq pre,$0), as predicted.
 *   f10exact (this file's form)                               : 4 = 2 diffs
 * So there is NO semantics-correct spelling of this topology: mask-fold is
 * the only active ingredient and it is exactly the H-s1-1 counter-split
 * axis, dead by the guard-fold constraint.
 */
void func_80044098(s16 a0) {
    s32 *v1;
    s32 a4;
    s32 *a6;

    v1 = D_80103608[a0];
    a4 = *(v1 - 1);
    a6 = v1;
    a6 = a6 - 1;
    if (a4 & 0x8000) {
        *(v1 - 1) = a4 & 0x7FFF;   /* BUG vs original: a4 left unmasked */
        a4 = a4 - 1;
        if (a4 != -1) {
            do { do { *v1 -= (s32)a6; v1++; a4--; } while (a4 != -1); } while (0);
        }
    }
}
