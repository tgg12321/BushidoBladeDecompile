void func_80033D38(void) {
    s32 n;
    s32 k;

    for (n = 3; n > 0; n--) {
        if (D_80106A50.times[n - 1].unk_4 < D_800A3858) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            D_80106A50.times[k] = D_80106A50.times[k - 1];
        }
        D_80106A50.times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        D_80106A50.times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        D_80106A50.times[n].unk_4 = D_800A3858;
    }
}
