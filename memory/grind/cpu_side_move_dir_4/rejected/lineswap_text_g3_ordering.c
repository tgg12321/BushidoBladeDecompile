/* s23 PERM_LINESWAP_TEXT find output-40-8 (weighted permuter score 40 = g3-basin
 * per s22 correlation). Novel ordering: v0/arg5 hoisted before final t0 finish,
 * with pp assignment interleaved in the t0 chain. Assumed sandbox masked=6 (g3
 * basin) per s22 correlation (weighted 40 <-> masked 6). Not measured this
 * session; g3 dupe of the s22 output-40-6/7 finds. */
  {
    s32 arg5;
    s32 t0;
    void **pp;
    tslTm2LoadImage_2(&D_800161B8);
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    v0 <<= 2;
    arg5 = *((s32 *) (v0 + ((s32) tbl_125c)));
    t0 *= 4;
    pp = (void **) (&D_800F19C0);
    t0 = (s32) (((u8 *) tbl_125c) + t0);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *((s32 *) t0), arg5);
  }
