s32 func_80021904(s32 a0) {
    s32 offset = a0 * 1100;
    s16 v1 = *(s16 *)((u8 *)&D_80101F12 + offset);
    s16 v0 = *(s16 *)((u8 *)&D_80101F4E + offset);
    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;
}
