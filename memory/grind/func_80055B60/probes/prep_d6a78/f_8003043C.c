void func_8003043C(void) {
    s32 i = 0;
    s16 neg = -1;
    u8 val = 0xFF;
    do {
        D_80106A78[i].unk_02 = neg;
        D_80106A78[i].unk_0A = val;
    } while (++i < 12);
}
