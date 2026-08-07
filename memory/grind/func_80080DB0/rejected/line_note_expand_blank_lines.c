/* s56 H3-inverse KILLED: 4 residual-window statements each on its own line, blank lines between.
   masked=2 INERT vs h5 baseline. Bytes identical.
   Together with the single-line probe, brackets the physical-line-layout axis
   as fully masked from residual pair-swap in production build (no -g).
*/
    {
      s32 arg5;
      s32 t0;
      void **pp;
      t0 = idx_1494[0];
      v0 = idx_1494[1];
      pp = (void **)&D_800F19C0;
      t0 *= 4;

      t0 = (s32)((u8 *)tbl_125c + t0);

      v0 <<= 2;

      arg5 = *(s32 *)(v0 + (s32)tbl_125c);
      debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)t0, arg5);
    }
