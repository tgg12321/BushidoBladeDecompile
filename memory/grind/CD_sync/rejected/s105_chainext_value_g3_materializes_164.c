  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_125c);
    arg5 = (s32)((u8 *)arg5 + ((s32)tbl_125c - (s32)D_800A125C)); /* FAKE: combine-foldable chain-extender (link-constant zero delta), mechanism: flow.c reg_n_refs before combine.c symbol fold, lever-exhaustion: memory/grind/CD_sync/hypotheses.md s101/s102 */
    t0 <<= 2;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)(t0 + (s32)tbl_125c), arg5);
  }
