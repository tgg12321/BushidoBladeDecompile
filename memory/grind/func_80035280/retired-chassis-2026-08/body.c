void func_80035280(void) {
    s32 *p;
    u8 *src1;
    u8 *src2;
    int new_var;
    u8 *dst;
    s32 i;
    s32 j;
    s32 v;
    volatile u8 *pbit;

    p = func_80077D00();
    pbit = &D_80106A73;
    v = p[8];
    v = v & (~1);
    v = v | ((*pbit) & 1);
    p[8] = v;
    v = v & (~2);
    v = v | ((*pbit) & 2);
    p[8] = v;
    v = v & (~4);
    v = v | ((*pbit) & 4);
    p[8] = v;

    src1 = (&D_80106A73) - 3;
    for (i = 0; i < 3; i++) {
        ((u8 *)p)[0x17 + i] = src1[i];
        ((u8 *)p)[0x1D + i] = src1[i];
    }

    src2 = (u8 *)(&D_80106A58);
    dst = (u8 *)p;
    for (j = 0; j < 3; j++) {
        dst[0x21] = (*((s32 *)(src2 + 4))) / 1800;
        new_var = ((*((s32 *)(src2 + 4))) / 30) % 60;
        v = new_var;
        dst[0x22] = v;
        dst[0x23] = (((*((s32 *)(src2 + 4))) % 30) * 100) / 30;
        dst[0x24] = src2[0];
        dst += 4;
        src2++;
        src2--;
        src2 += 8;
        j++;
        j--;
    }
}
