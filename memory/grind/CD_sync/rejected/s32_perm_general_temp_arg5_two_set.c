/* s32 PERM_GENERAL fresh-seed novel find (output-40-9, weighted=40 = g3 basin).
 * Mutation: hoist `arg5 = *(s32*)(v0+tbl)` into two-set form via `s32 temp;`
 * fn-scope declaration, then `temp = *(s32*)(v0+tbl); arg5 = temp;`.
 * Score class: weighted=40 correlates with masked=6 (g3 basin) per s22/s23
 * empirical correlation (documented in hypotheses.md L479). Fresh-set `temp`
 * pseudo splits arg5's single-set into two SETs, but the resulting form falls
 * into g3-basin (direct-LSHIFT p101 in-place path — s7 CONFIRMED)
 * not h5-basin, so pri(arg5) LAUNCH regresses. Same family as s3-V6 (arg4_named)
 * and s11 M1-secondary (t_alias opaque-carrier two-SET). Novel spelling, same
 * class result. */
{
    s32 arg5;
    s32 t0;
    void **pp;
    s32 temp; /* PERM_GENERAL split — fn-scope actually */
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    temp = *(s32 *)(v0 + (s32)tbl_125c);
    arg5 = temp;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
}
