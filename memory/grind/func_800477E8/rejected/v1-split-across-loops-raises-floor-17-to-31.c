/* REJECTED (s1, 2026-07-30) — splitting the shared `v1` local into two
 * short-lived locals, one per loop, on the theory that its whole-function live
 * range was an m2c artifact starving its allocno priority for $v1.
 *
 * MEASURED: sandbox --disable all  17 -> 31   (build_insns still 170)
 *
 * This is a strong POSITIVE finding in the other direction: target's allocation
 * requires ONE variable shared across loop1 and loop2, exactly as HEAD has it.
 * Never re-propose splitting it; any rederive must preserve the sharing.
 *
 * The delta applied to the HEAD body (see ../candidate.c):
 */

/*  declarations:  */
    s32 v1;
    s32 v1b;   /* <-- added */

/*  loop1 unchanged (uses v1) ... */

/*  loop2, was `v1 = a2;` / `*p = v1;`  */
outer2:
    v1b = a2;
    a0 = 0x10;
    p = ptr + 0x10;
inner2:
    *p = v1b;
    a0--;
    p--;
    if (a0 >= 0) goto inner2;
