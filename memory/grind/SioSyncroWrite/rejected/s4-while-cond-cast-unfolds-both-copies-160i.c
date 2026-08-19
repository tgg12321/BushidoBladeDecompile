/* REJECTED (s4) — 160 insns vs target 159.
 * Chassis: the s3 `while (D_800F1AF4 != 0) { ... }` loop with the condition
 * spelled `*(volatile s32 *)&D_800F1AF4 != 0`.
 * WHY IT IS DEAD: expand_end_loop duplicates a `while` condition VERBATIM to
 * the loop top, so the cast un-folds BOTH copies (entry test AND bottom test).
 * Target has the entry test FOLDED (lui;lw) and only the bottom test un-folded
 * (lui;addiu;lw) — two copies of one condition can never differ, so no
 * spelling of a `while` condition can ever reproduce target. This is the
 * measurement that killed the entire while-chassis and forced the
 * guard + for(;;) + block-scoped-alias-break form now in candidate.c.
 * Same result for (&D_800F1AF4)[0] != 0 (160i) and for a volatile-qualified
 * pointer statement-expression (160i).                                      */
    while (*(volatile s32 *)&D_800F1AF4 != 0) {
        /* ... s3 body unchanged ... */
    }
