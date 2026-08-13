/* REJECTED (grind session 9c, PERMUTER modality, 2026-08-13) — NOT proposed.
 *
 * WHAT WAS RUN.  A fresh decomp-permuter campaign on a brand-new offset-0
 * workspace built from the OA chassis
 * (rejected/maintained-index-nonfold-reset-costs-one-insn.c — the maintained-
 * index do/while body that is 3 / 79, i.e. the ONLY chassis the grind has that
 * gets the whole inner-loop top right with no scheduling lever, at the price of
 * emitting `j = 0` twice).  Workspace tmp/grind/func_800645B0/s9c/ws_oa, built
 * by the same s9c recipe; permuter base score 160, validated against the known
 * duplicate-`move a0,zero` diff.
 *
 * RESULT: 21,451 iterations / 24.2 min, NO score-0 find; best 60.
 *
 * WHY THE BEST FIND IS DEAD.  output-60-1 reaches 60 by DELETING the pre-loop
 * `j = 0;` initialiser (and parking a dead `j = 0;` after the `break;` where it
 * can never execute).  `j` is then read uninitialised on the very first inner
 * iteration: the mutation buys its instruction by breaking the function's
 * semantics, exactly like the already-banked
 * rejected/permuter-semantics-breaking-j-mutations.c.  Not proposable at any
 * score.
 *
 * WHAT IT CONFIRMS.  The permuter, given ~21k random attempts, found exactly
 * ONE way to delete OA's extra instruction — deleting the initialiser — which
 * is precisely the structural conclusion session 8 reached by hand (sweep28's
 * six placements OA/OB/OC/OD/OE/OF + PA/PB all pay the duplicate `j = 0`, and
 * the only placement that does not is the natural `for (j = 0; ...)` init,
 * which then lets cse fold `i + j` to `move $s0,$s3`).  The OA basin is
 * therefore measured dead on the permuter axis: its one instruction of slack is
 * structural, and no semantics-preserving mutation in the neighbourhood removes
 * it.
 *
 * Artifacts: tmp/grind/func_800645B0/s9c/ws_oa/{campaign.log,campaign_meta.json,
 * output-60-1}.
 */
#if 0
    /* the find, minus the cosmetic reformatting: note the missing `j = 0;`
       before the outer loop, and the unreachable one after `break;`. */
    D_800F10EC = 1;
    for (i = 0; i < 0xF; i += 4, j = 0) {
        idx = i + j;                 /* first pass reads an uninitialised j */
        do {
            /* ... OA body ... */
                break;
                j = 0;               /* dead */
            j += 1;
            idx = i + j;
        } while (j < 4);
    }
#endif
