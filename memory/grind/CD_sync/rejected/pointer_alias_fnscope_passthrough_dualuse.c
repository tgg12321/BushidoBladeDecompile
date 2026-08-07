/* s101 probe D: pointer-alias-fake-exception; fn-scope alias assigned in prologue, dual-use in block.
   Result: masked=2 build_insns=160 INERT (byte-neutral even with fn-scope livelen; combine still folds both refs). */
  s32 *tbl_alias;
  ...
  tbl_125c = D_800A125C;
  tbl_alias = tbl_125c; /* FAKE: prologue-time alias init */
  ...
    t0 = (s32)((u8 *)tbl_alias + t0);
    arg5 = *(s32 *)(v0 + (s32)tbl_alias);
