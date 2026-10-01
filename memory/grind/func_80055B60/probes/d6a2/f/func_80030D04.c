void func_80030D04(void) {
    s32 i = 0;
    do {
        if (D_80106A78[i].unk_02 >= 0x12 && D_80106A78[i].unk_02 < 0x1E) {
            D_80106A78[i].unk_02 = -1;
        }
    } while (++i < 12);
}
