/* s23 PERM_LINESWAP_TEXT find output-55-3 (weighted permuter score 55).
 * Novel ordering: v0/arg5 chain hoisted entirely before the t0 chain, with
 * tslTm2LoadImage_2 at the top and pp assignment in the middle.
 * Sandbox --disable all measurement: masked=7, target_insns=160, build_insns=160.
 * Novel intermediate basin (+5 vs h5=2). Not h5, not g3 (6). PERM_LINESWAP_TEXT
 * reached an ordering that PERM_LINESWAP could not (crossed the tslTm2LoadImage_2
 * boundary), but that ordering scores WORSE than h5 - the arg5-before-t0
 * head-order flip disrupts the h5 alignment (mirrors s2 head-load-flip finding).
 * SEE ALSO: rejected/lineswap_text_g3_ordering.c for the g3-basin dupe find. */
  {
    s32 arg5;
    s32 t0;
    void **pp;
    tslTm2LoadImage_2(&D_800161B8);
    v0 = idx_1494[1];
    v0 <<= 2;
    t0 = idx_1494[0];
    pp = (void **) (&D_800F19C0);
    arg5 = *((s32 *) (v0 + ((s32) tbl_125c)));
    t0 *= 4;
    t0 = (s32) (((u8 *) tbl_125c) + t0);
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *((s32 *) t0), arg5);
  }
