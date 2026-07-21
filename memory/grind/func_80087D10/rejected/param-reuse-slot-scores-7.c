/* REJECTED s1: C-level param reuse for slot — scores 7 (baseline 6, old form 8).
 * The explicit a0 clobber creates the sh->andi anti-dep chain at C level and raises the
 * store cluster's scheduling priority even higher; direction is wrong. */
s16 func_80087D10(s32 a0) {
    u8 *base;
    s32 slot;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    D_80102806 = a0;
    a0 = (a0 & 0xFF00) >> 8;
    slot = a0;
    base += slot * 176;
    return *(s16 *)(base + 0x58);
}
