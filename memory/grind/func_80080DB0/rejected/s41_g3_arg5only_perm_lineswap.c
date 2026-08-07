/* s41 rejected: g3-chassis arg5-side-only PERM_LINESWAP (s19 frontier #3).
 *
 * Chassis: g3 basin (direct-LSHIFT on both t0 and v0; p101 multi-set in-place).
 * Base score confirmed at iteration 1 = 40 (g3 baseline).
 * PERM_LINESWAP wraps ONLY the two arg5-side statements:
 *     v0 <<= 2;
 *     arg5 = *(s32*)(v0 + (s32)tbl_125c);
 * Total deterministic space: 2!=2 orderings. Enumeration exhausted:
 *   iter 1 (base ordering): score=40 (g3 baseline)
 *   iter 2 (swap ordering): score=140 (regression; arg5 dereferences unshifted v0)
 * Zero sub-40 finds. Campaign artifact:
 *   tmp/grind/cpu_side_move_dir_4/s41/perm_g3_arg5only/{base.c,campaign.log,campaign_meta.json}
 *
 * This closes s19 frontier #3 (elevated but never executed prior to s41): the
 * g3-chassis arg5-side-only permuter direction. Combined with s5 (unrestricted
 * g3 permuter, 9040 iters), s14 (g3-drift directed, 23427 iters), s22 (h5
 * LINESWAP, 5040), s23 (h5 LINESWAP_TEXT, 20429), s31 (prologue LINESWAP, 120),
 * s32 (h5 PERM_GENERAL, 35338), s40 (h5 wider-scope PERM_GENERAL, 1738), and
 * this s41 (g3 arg5-only LINESWAP, 2): every non-trivial permuter scope and
 * chassis on this function has been searched with zero sub-basin finds.
 */
{
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    PERM_LINESWAP(
    v0 <<= 2;
    arg5 = PERM_GENERAL(*(s32 *)(v0 + (s32)tbl_125c));
    )
    t0 <<= 2;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)(t0 + (s32)tbl_125c), arg5);
}
