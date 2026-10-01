void func_80030524(void) {
    s32 i = 0;
    s32 neg = -1;
    do {
        if (D_80106A78[i].unk_02 != neg) {
            if (D_80106A78[i].unk_08 != 0) {
                D_80106A78[i].unk_02 = neg;
            }
        }
    } while (++i < 12);
}
