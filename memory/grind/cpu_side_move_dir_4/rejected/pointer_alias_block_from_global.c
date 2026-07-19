/* s101 probe B: pointer-alias-fake-exception; block-scope alias initialized from global D_800A125C.
   Result: masked=16 build_insns=161 (+14 / +1insn) KILLED — independent lui/addiu materialization added and register web catastrophically rotated. */
    s32 *tbl_alias = D_800A125C; /* FAKE: independent-source alias */
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_alias);
