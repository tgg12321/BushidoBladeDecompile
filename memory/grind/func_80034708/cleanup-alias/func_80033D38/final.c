void func_80033D38(void) {
    /* FAKE: pointer to the record, admitted on SOTN precedent (Q50, Q53); mechanism: its
     * register (t1) is the base of every times[] access and of the shift loop's pointer;
     * exhaustion: direct D_80106A50.times[] 34 (each access lui/addu/%lo, 53/47 insns),
     * direct for-loop 34, member-wise shift 41, times-array pointer 11 (base + 8);
     * memory/grind/func_80034708/evidence.md [s10] */
    FileRecord *rec = &D_80106A50; /* SOTN: src/dra/4CE2C.c:63 @db41b28 */
    s32 n = 3;
    s32 j;
    s32 k;

    while (1) {
        j = n - 1;
        if (rec->times[j].unk_4 < D_800A3858) {
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
            rec->times[k] = rec->times[k - 1];
        }
        rec->times[n].unk_0 = (u8)g_practice_menu_table[0].unk_0A;
        rec->times[n].unk_1 = (u8)g_practice_menu_table[0].unk_0E;
        rec->times[n].unk_4 = D_800A3858;
    }
}
