void func_80033D38(void) {
    FileRecord *rec = &D_80106A50;
    s32 n;
    s32 k;

    for (n = 3; n > 0; n--) {
        if (rec->times[n - 1].unk_4 < D_800A3858) {
            break;
        }
    }
    D_800A38E9 = (u8)n;
    if (n < 3) {
        for (k = 2; k > n; k--) {
            rec->times[k] = rec->times[k - 1];
        }
        rec->times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        rec->times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        rec->times[n].unk_4 = D_800A3858;
    }
}
