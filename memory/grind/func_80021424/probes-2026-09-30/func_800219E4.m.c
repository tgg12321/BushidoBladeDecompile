s32 func_800219E4(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    return D_80102760 + D_800A3860[v0]->f16 * 2;
}
