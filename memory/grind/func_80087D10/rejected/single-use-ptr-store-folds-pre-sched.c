/* REJECTED s1: single-use pointer store — cse folds ptr into the MEM before sched1,
 * bytes identical to the direct store (score 6, same as candidate). The CAC pointer-alias
 * mechanism needs TWO accesses to keep the pointer live; it does not transfer here.
 * Artifact: tmp/grind/func_80087D10/s1/built_v3_single_use_ptr.s */
s16 func_80087D10(s32 a0) {
    u8 *base;
    s32 slot;
    s16 *ptr;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    ptr = &D_80102806;
    *ptr = a0;
    slot = (a0 & 0xFF00) >> 8;
    base += slot * 176;
    return *(s16 *)(base + 0x58);
}
