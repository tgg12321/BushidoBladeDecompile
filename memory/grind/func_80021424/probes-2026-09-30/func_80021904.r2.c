s32 func_80021904(s32 a0) {
    s16 v1 = g_practice_menu_table[a0].unk_4A;
    s16 v0 = g_practice_menu_table[a0].unk_86;
    return D_801027B0[v1][0] + D_800A3860[v1]->f4E[v0] * 2;
}
