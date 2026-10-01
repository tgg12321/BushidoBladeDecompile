s32 func_80021A3C(s32 a0, s32 a1) {
    s32 offset = a0 * 1100;
    s16 v0 = *(s16 *)((u8 *)&D_80101F12 + offset);
    return D_80102760 + D_800A3860[v0]->f18[a1] * 2;
}
