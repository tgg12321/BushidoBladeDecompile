/* REJECTED — measured session 2 (2026-08-17).
 *
 * Why dead: replacing the search loop's byte-offset accesses with struct
 * accesses through the SUBSLOT view drops the build from 76 to 72 instructions
 * and raises the honest sandbox distance from 0 to 11. Target re-materializes
 * %hi(SYM) + an offset register for every access OUTSIDE the copy loop; the
 * struct-indexed form lets GCC strength-reduce / CSE a single base and folds
 * the two loads. The byte-offset spelling (`off = i << 4`, then
 * `*(s16 *)((u8 *)D_800EED10 + off)`) — which is also the idiom every other
 * function in src/text1a_c.c uses on this table — is load-bearing.
 *
 * Only the two changed lines are shown; everything else is candidate.c.
 */

    for (i = 0; i < D_800A33AC; i++) {
        off = i << 4;
        if (SUBSLOT[i + 1].id == a0) {                 /* was: *(s16 *)((u8 *)D_800EED10 + off) */
            func_80045294(i + 1, -SUBSLOT[i + 1].amt); /* was: -*(s32 *)((u8 *)&D_800EED18 + off) */
            /* ... rest unchanged ... */

/* sandbox --disable all: score 11, build_insns 72 (target 76). */
