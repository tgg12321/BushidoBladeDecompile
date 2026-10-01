void func_80030D04(void) {
    s32 i;
    for (i = 0; i < 12; i++) {
        if ((u16)(D_80106A78[i].unk_02 - 0x12) < 12) {
            D_80106A78[i].unk_02 = -1;
        }
    }
}
