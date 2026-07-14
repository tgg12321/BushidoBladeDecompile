/* Best known form — sandbox 0 (verified s2, 2026-07-14). This is the form
   committed on main. Judge-flagged pending ruling: pointer-alias-fake-exception
   annotation request outstanding (see hypotheses.md s2). All pointer-free
   spellings measured at distance 12 (rejected/). */
s16 func_80087CAC(s32 a0, s16 *a1, s16 *a2) {
    u8 *base;
    s16 *ptr;
    s32 slot;
    u8 *p;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    ptr = &D_80102806;
    *ptr = a0;
    slot = (a0 & 0xFF00) >> 8;
    p = base + slot * 176;
    *a1 = *(u16 *)(p + 0x58);
    *a2 = *(u16 *)(p + 0x5A);
    return *ptr;
}
