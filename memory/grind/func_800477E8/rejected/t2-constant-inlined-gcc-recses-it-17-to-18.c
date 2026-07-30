/* REJECTED (s1, 2026-07-30) — removing the `t2` local entirely and writing the
 * 0x2C00 literal at both store sites, to eliminate one contestant for $t1 and
 * let `t1val` take it.
 *
 * MEASURED: sandbox --disable all  17 -> 18   (build_insns still 170)
 *
 * GCC re-CSE'd the two literals straight back into a single pseudo — both build
 * and target still emit exactly one `li ...,11264` — so the competing allocno
 * did not go away and tie A's seating was byte-for-byte unchanged. The only
 * effect was that the re-materialised constant scheduled one slot later than
 * target (`move a0,zero` and `li t1,11264` traded places), adding one diff.
 *
 * Conclusion: constant SPELLING is not the lever for tie A. The lever, if any,
 * is the reference-count / live-length ratio that global.c's allocno_compare
 * uses (see hypotheses.md F1).
 *
 * Delta applied to the HEAD body: `s32 t2;` declaration deleted,
 * `t2 = 0x2C00;` deleted, and both store sites rewritten:
 */

        /* in the a3 >= 5 arm */
        *s0 = 0x2C00;
        /* ... and in the else arm */
        *s0 = 0x2C00;
