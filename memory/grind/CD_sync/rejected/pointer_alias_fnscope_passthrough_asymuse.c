/* s101 probe E: pointer-alias-fake-exception; fn-scope alias, asymmetric use (t0 keeps tbl_125c, arg5 uses tbl_alias).
   Result: masked=10 build_insns=163 (+8 / +3insn) KILLED — fn-scope alias survives fold when only one downstream use exists inside the block, extra prologue materialization + local-alloc rotation adds 3 insns. */
  s32 *tbl_alias;
  ...
  tbl_125c = D_800A125C;
  tbl_alias = tbl_125c;
  ...
    t0 = (s32)((u8 *)tbl_125c + t0);
    arg5 = *(s32 *)(v0 + (s32)tbl_alias);
