s16 func_80087D10(s32 a0) {
    u8 *base;
    s32 slot;
    base = (u8 *)((s32 *)&D_80106F28)[(u8)a0];
    D_80102806 = a0;
    slot = (a0 & 0xFF00) >> 8;
    base += slot * 176;
    return *(s16 *)(base + 0x58);
}
