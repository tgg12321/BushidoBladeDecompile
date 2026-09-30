void func_80033D38(void) {
    FileTimeRec *t = D_80106A50.times;
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (t[j].unk_4 < D_800A3858) {
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
            t[k] = t[k - 1];
        }
        t[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        t[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        t[n].unk_4 = D_800A3858;
    }
}
