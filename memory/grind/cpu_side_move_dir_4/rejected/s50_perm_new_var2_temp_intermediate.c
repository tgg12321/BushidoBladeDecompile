/* s50 permuter find (weighted=40, masked=2 — same-floor, not improvement).
 * Novel structural axis vs h5 candidate.c: (a) hoists `&D_800F19C0` out to a
 * fn-scope `void **new_var2;` initialised before the inline block, and pp is
 * initialized from new_var2 instead of directly from &D_800F19C0. (b) introduces
 * a `s32 temp;` intermediate: `temp = *(s32*)(v0+(s32)tbl_125c); arg5 = temp;`
 * instead of assigning arg5 directly.
 *
 * Sandbox: masked=2 (h5 baseline unchanged). Weighted=40 is a register-name
 * reshuffle the masked scorer normalizes out; the honest distance is the same.
 * Cheat-vetting: `temp` is a load intermediate with no semantic purpose (arg5
 * is a load-and-pass); `new_var2` is a pointer-alias fn-scope handle with the
 * same coercion intent as pp — the alias-rename family expanded catalog spelling.
 * Not a legitimate closing form even if it had improved the floor.
 */
  do_timeout:
  tslTm2LoadImage_2(&D_800161B8);

  {
    s32 arg5;
    s32 t0;
    void **pp;
    void **new_var2;
    s32 temp;
    new_var2 = &D_800F19C0;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **) new_var2;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    temp = *(s32 *)(v0 + (s32)tbl_125c);
    arg5 = temp;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
  }
