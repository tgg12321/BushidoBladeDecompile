void func_800768DC(s32 arg0, s32 arg1, s16 *arg2, s32 arg3) {
    s16 i;
    s16 j;

    if (SELWORK->f10.half[arg3] != 0) {
        return;
    }
    if (arg0 & (0xA000 << (arg3 * 16))) {
        func_8005C650(0, 0x7F, 0x7F);
    }
    switch (func_800692C0((u32 *)&arg0, arg3, SELWORK->f40[arg3], D_800A35D0[arg3]) & 0xFF) {
    case 1:
        SELWORK->f5C[arg3]++;
        if (SELWORK->f5C[arg3] >= SELWORK->f60[arg3]) {
            SELWORK->f5C[arg3] = 0;
        }
        break;
    case 2:
        SELWORK->f5C[arg3]--;
        if (SELWORK->f5C[arg3] < 0) {
            SELWORK->f5C[arg3] = SELWORK->f60[arg3] - 1;
        }
        break;
    }

    if (arg0 & (0x40 << (arg3 * 16))) {
        if (SELWORK->f3C.half[arg3] < SELWORK->f65 + 3) {
            func_8005C650(1, 0x7F, 0x7F);
            SELWORK->f7E[arg3][SELWORK->f3C.half[arg3]] = SELWORK->f48[arg3][SELWORK->f5C[arg3]];
            if (SELWORK->f3C.half[arg3] == SELWORK->f65 + 2) {
                SELWORK->f14.half[arg3] = 4;
                return;
            }
            SELWORK->f60[arg3]--;
            for (i = SELWORK->f5C[arg3]; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = SELWORK->f48[arg3][i + 1];
            }
            SELWORK->f5C[arg3] = 0;
            SELWORK->f3C.half[arg3]++;
        }
    } else if (arg0 & (0x10 << (arg3 * 16))) {
        func_8005C650(2, 0x7F, 0x7F);
        if (SELWORK->f3C.half[arg3] == 0) {
            SELWORK->f10.half[arg3] = 3;
            SELWORK->f18[arg3] = 2;
            SELWORK->f38[arg3] = SELWORK->f65 + 2;
            D_8009BCE4[arg2[SELWORK->f38[arg3]]] &= ~(4 << arg3);
            SELWORK->f60[arg3] = 5;
            for (i = 0; i < SELWORK->f60[arg3]; i++) {
                SELWORK->f48[arg3][i] = i;
            }
            if (arg1 != 0) {
                SELWORK->f48[arg3][4] = 5;
            }
            return;
        }
        /* Put the last taken entry back into the list in sorted position. */
        for (i = 0; i < SELWORK->f60[arg3]; i++) {
            if (SELWORK->f48[arg3][i] > SELWORK->f7E[arg3][SELWORK->f3C.half[arg3] - 1]) {
                for (j = SELWORK->f60[arg3] - 1; j >= i; j--) {
                    SELWORK->f48[arg3][j + 1] = SELWORK->f48[arg3][j];
                }
                SELWORK->f48[arg3][i] = SELWORK->f7E[arg3][SELWORK->f3C.half[arg3] - 1];
                break;
            }
            if (i == SELWORK->f60[arg3] - 1) {
                SELWORK->f48[arg3][i + 1] = SELWORK->f7E[arg3][SELWORK->f3C.half[arg3] - 1];
            }
        }
        SELWORK->f60[arg3]++;
        SELWORK->f3C.half[arg3]--;
    }
}
