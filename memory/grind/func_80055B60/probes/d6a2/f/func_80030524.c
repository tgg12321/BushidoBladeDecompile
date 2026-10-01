void func_80030524(void) {
    s32 i = 0;
    do {
        if (D_80106A78[i].unk_02 != -1) {
            if (D_80106A78[i].unk_08 != 0) {
                D_80106A78[i].unk_02 = -1;
            }
        }
    } while (++i < 12);
}
