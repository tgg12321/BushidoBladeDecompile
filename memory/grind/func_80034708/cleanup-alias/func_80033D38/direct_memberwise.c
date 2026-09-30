void func_80033D38(void) {
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (D_80106A50.times[j].unk_4 < D_800A3858) {
            break;
        }
        n = j;
        if (n <= 0) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            D_80106A50.times[k].unk_0 = D_80106A50.times[k - 1].unk_0;
            D_80106A50.times[k].unk_1 = D_80106A50.times[k - 1].unk_1;
            D_80106A50.times[k].unk_4 = D_80106A50.times[k - 1].unk_4;
        }
        D_80106A50.times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        D_80106A50.times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        D_80106A50.times[n].unk_4 = D_800A3858;
    }
}
