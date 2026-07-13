/* REJECTED (s2, structural) — sandbox --disable all = 9 (93/93 insns). WORSE than
 * the plain direct spelling (6).
 *
 * The obvious no-pointer alternative: stage the sign-extended index through an
 * ordinary local, re-read after the call (semantically the reload is real — the
 * call can change D_8010277C).  This still symbol-addresses BOTH lb's (no address
 * pseudo is created — same mechanism as direct-cast-no-address-cache-scores-6.c)
 * AND it perturbs the surrounding schedule/allocation, costing 3 more masked diffs.
 * Staging the VALUE is not a substitute for caching the ADDRESS.
 */
        s32 i;

        EndADRSound();
        game_StageCleanup(D_800A36A4, s2);
        func_8002906C();
        func_8005BDF0();

        i = (s8)D_8010277C;
        s1 = func_8005BA8C(s2, D_800A36A4, *(&D_8008E5A8 + i), *(&D_8008E5A8 + D_8010277D));

        D_800A390E = D_800A36A4;
        i = (s8)D_8010277C;
        D_800A30FC = *(&D_8008E5A8 + i);
        D_800A30FD = *(&D_8008E5A8 + D_8010277D);
