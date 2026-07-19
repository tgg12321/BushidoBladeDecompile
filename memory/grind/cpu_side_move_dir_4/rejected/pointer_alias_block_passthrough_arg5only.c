/* s101 probe A: pointer-alias-fake-exception; block-scope pass-through, asymmetric use (arg5 only).
   Result: masked=2 build_insns=160 INERT (byte-neutral, combine folds alias back to tbl_125c pseudo — no p79 nrefs split). */
    s32 *tbl_alias = tbl_125c; /* FAKE: pointer-alias-fake-exception per 2026-07-01 sanction */
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_125c + t0);
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_alias);
