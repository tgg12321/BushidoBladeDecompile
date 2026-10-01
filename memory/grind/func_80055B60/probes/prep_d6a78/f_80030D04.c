void func_80030D04(void) {
    s32 i = 0;
    s32 neg = -1;
    do {
        if ((u32)((u16)D_80106A78[i].unk_02 - 0x12) < 12u) {
            D_80106A78[i].unk_02 = neg;
        }
    } while (++i < 12);
}
