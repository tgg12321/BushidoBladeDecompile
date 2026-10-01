s32 func_80021A3C(s32 a0, s32 a1) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s32 base = *(s32 *)((u8 *)&D_800A3860 + v0 * 4);
    u16 idx = *(u16 *)(base + a1 * 2 + 0x18);
    return D_80102760 + idx * 2;
}
