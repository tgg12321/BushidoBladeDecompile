  {
    s32 arg5;
    s32 t0;
    void **pp;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + ((s32)idx_1494 + ((s32)D_800A125C - (s32)&D_800A1494))); /* FAKE: combine-foldable chain-extender via the idx_1494 link-constant delta, mechanism: flow.c reg_n_refs recorded before the combine.c symbol fold, lever-exhaustion: memory/grind/CD_sync/hypotheses.md s101/s102/s105 */
    t0 <<= 2;
    debug_printf(&D_800161C8, *pp, D_800A11DC[D_800A11D5], *(s32 *)(t0 + (s32)tbl_125c), arg5);
  }
