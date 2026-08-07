/* s101 probe C: pointer-alias-fake-exception; block-scope pass-through, symmetric use (both t0 and arg5 routed via alias).
   Result: masked=2 build_insns=160 INERT (byte-neutral, combine folds both dual-uses back to tbl_125c pseudo — no p79 nrefs split). */
    s32 *tbl_alias = tbl_125c;
    t0 = idx_1494[0];
    v0 = idx_1494[1];
    pp = (void **)&D_800F19C0;
    t0 *= 4;
    t0 = (s32)((u8 *)tbl_alias + t0);
    v0 <<= 2;
    arg5 = *(s32 *)(v0 + (s32)tbl_alias);
