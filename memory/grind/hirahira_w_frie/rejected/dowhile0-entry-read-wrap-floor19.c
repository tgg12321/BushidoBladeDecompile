/* REJECTED s2b (2026-07-17, structural): sanctioned do-while(0) wrapper around
 * the entry `v1 = *offsets; offsets++;` read.
 * Hypothesis: loop notes might fence sched1's entry region in a way that
 * touches the arg-copy pair order.
 * MEASURED: sandbox --disable all = 19, build_insns 58 vs 59 — strictly worse;
 * the injected loop notes perturb the REAL loop's note structure (reorg
 * delay-slot fill and preheader placement), losing an insn.
 * KILLED. Also mechanism-inert for the pair by construction: both entry
 * copies are emitted adjacently by expand_function_start BEFORE any statement,
 * so no statement-level wrapper can place a fence between them.
 * (Fragment shown; body otherwise the clean+stop floor-4 form.)
 */
    do { /* FAKE: probe */
        v1 = *offsets;
        offsets++;
    } while (0);
    if (v1 == -2) { ... }
