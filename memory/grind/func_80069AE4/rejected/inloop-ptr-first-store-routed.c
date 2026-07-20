/* REJECTED s1 (recon, 2026-07-20): second real use for the pointer via routing
 * ONLY the first store through it (rejected-bank form (a) routed ALL stores).
 * Score 5 (worse than floor 2).
 * Why dead: cse folds the store's address use 0(dst) -> 24(sp) anyway (emitted
 * `sw v1,24(sp)`), so dst STILL dies at the arg load and combine still merges
 * the addiu at the call site; meanwhile the longer-lived load temp perturbs RA
 * (lw into $v1 instead of $v0) and the sw migrates next to the jal. Kills the
 * whole "second use via store-routing" family: every store use folds to
 * sp-form, so the pointer always dies at the arg load.
 */
    do {
        s32 *dst = &s.sp18;
        s32 v = *q;
        q++;
        *dst = v;
        s.sp1C = v + 0xC;
        s.sp20 = arg0[5];
        arg0[5] = func_8007352C((s32)dst);
        i++;
    } while (i < 3);
