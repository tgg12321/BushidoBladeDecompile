void func_80034F88(void) {
    s32 *p;
    volatile u8 *pbit;
    u8 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    pbit = &D_80106A73;
    *pbit &= 0xF8;

    do {
        c = p[8] & 1;
        val = *pbit;
        val2 = c ? (u8)(val | 1) : val;
        *pbit = val2;
    } while (0);

    do {
        c = p[8] & 2;
        val = *pbit;
        val2 = c ? (u8)(val | 2) : val;
        *pbit = val2;
    } while (0);

    do {
        c = p[8] & 4;
        val = *pbit;
        val2 = c ? (u8)(val | 4) : val;
        *pbit = val2;
    } while (0);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + 0x17 + i);
    }
}
