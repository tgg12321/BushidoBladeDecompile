void func_800768DC(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 i;
    s16 j;

    if (SELWORK_800768DC->f10[arg3] != 0) {
        return;
    }
    if (arg0 & (0xA000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    switch (func_800692C0((u32 *)&arg0, arg3, SELWORK_800768DC->f40[arg3], (&D_800A35D0) + (arg3 * 2)) & 0xFF) {
    case 1:
        SELWORK_800768DC->f5C[arg3]++;
        if (SELWORK_800768DC->f5C[arg3] >= SELWORK_800768DC->f60[arg3]) {
            SELWORK_800768DC->f5C[arg3] = 0;
        }
        break;
    case 2:
        SELWORK_800768DC->f5C[arg3]--;
        if (SELWORK_800768DC->f5C[arg3] < 0) {
            SELWORK_800768DC->f5C[arg3] = SELWORK_800768DC->f60[arg3] - 1;
        }
        break;
    }

    if (arg0 & (0x40 << (arg3 * 16))) {
        if (SELWORK_800768DC->f3C[arg3] < SELWORK_800768DC->f65 + 3) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3]] = SELWORK_800768DC->f48[arg3][SELWORK_800768DC->f5C[arg3]];
            if (SELWORK_800768DC->f3C[arg3] == SELWORK_800768DC->f65 + 2) {
                SELWORK_800768DC->f14[arg3] = 4;
                return;
            }
            SELWORK_800768DC->f60[arg3]--;
            for (i = SELWORK_800768DC->f5C[arg3]; i < SELWORK_800768DC->f60[arg3]; i++) {
                SELWORK_800768DC->f48[arg3][i] = SELWORK_800768DC->f48[arg3][i + 1];
            }
            SELWORK_800768DC->f5C[arg3] = 0;
            SELWORK_800768DC->f3C[arg3]++;
        }
    } else if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK_800768DC->f3C[arg3] == 0) {
            SELWORK_800768DC->f10[arg3] = 3;
            SELWORK_800768DC->f18[arg3] = 2;
            SELWORK_800768DC->f38[arg3] = SELWORK_800768DC->f65 + 2;
            (&D_8009BCE4)[arg2[SELWORK_800768DC->f38[arg3]]] &= ~(4 << arg3);
            SELWORK_800768DC->f60[arg3] = 5;
            for (i = 0; i < SELWORK_800768DC->f60[arg3]; i++) {
                SELWORK_800768DC->f48[arg3][i] = i;
            }
            if (arg1 != 0) {
                SELWORK_800768DC->f48[arg3][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK_800768DC->f60[arg3]; i++) {
            if (SELWORK_800768DC->f48[arg3][i] > SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1]) {
                for (j = SELWORK_800768DC->f60[arg3] - 1; j >= i; j--) {
                    SELWORK_800768DC->f48[arg3][j + 1] = SELWORK_800768DC->f48[arg3][j];
                }
                SELWORK_800768DC->f48[arg3][i] = SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1];
                break;
            }
            if (i == SELWORK_800768DC->f60[arg3] - 1) {
                SELWORK_800768DC->f48[arg3][i + 1] = SELWORK_800768DC->f7E[arg3][SELWORK_800768DC->f3C[arg3] - 1];
            }
        }
        SELWORK_800768DC->f60[arg3]++;
        SELWORK_800768DC->f3C[arg3]--;
    }
}
