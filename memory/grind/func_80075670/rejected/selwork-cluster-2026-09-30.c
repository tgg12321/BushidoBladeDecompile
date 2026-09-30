void func_80075670(s32 arg0, s32 arg1) {
    s16 i;
    u8 *base;
    u8 *work;
    u8 *q;
    s16 *p;

    base = D_800A36A0;
    if (*(s32 *)(base + 0x10) != 0) {
        return;
    }
    if ((func_800692C0(&arg0, arg1, (s16 *)(base + (arg1 * 4 + 0x40)), (&D_800A35D0) + (arg1 * 2)) >> 16) != 0) {
        *(s16 *)(D_800A36A0 + 0x34) = 0;
        *(u8 *)(D_800A36A0 + arg1 + 0x68) = *(u8 *)(D_800A36A0 + arg1 + 0x68) + 1;
        base = D_800A36A0;
        *(u8 *)(base + arg1 + 0x68) &= 1;
        func_8005C650(0, 0x7F, 0x7F);
    }
    work = D_800A36A0;
    *(s16 *)(work + arg1 * 2 + 0x3C) = *(u8 *)(work + arg1 + 0x68);
    if (arg0 & (0x40 << (arg1 * 16))) {
        for (i = 0; i < 2; i++) {
            ((s16 *)(work + i * 2))[0x38 / 2] = 0;
            ((s16 *)(work + i * 2))[0x10 / 2] = 1;
            ((s16 *)(work + i * 2))[0x18 / 2] = 2;
        }
        q = D_800A36A0;
        *(u8 *)(q + ((arg1 + 1) & 1) + 0x68) = (*(u8 *)(q + arg1 + 0x68) + 1) & 1;
        func_8005C650(1, 0x7F, 0x7F);
        return;
    }
    if (arg0 & (0x10 << (arg1 * 16))) {
        if (arg1 != 0) {
            p = (s16 *)(work + 0x14);
        } else {
            p = (s16 *)(work + 0x16);
        }
        if (*p == 1) {
            for (i = 0; i < 2; i++) {
                ((s16 *)(work + i * 2))[0x38 / 2] = 0;
                ((s16 *)(work + i * 2))[0x10 / 2] = 3;
                ((s16 *)(work + i * 2))[0x18 / 2] = 0;
            }
            func_8005C650(2, 0x7F, 0x7F);
        } else {
            func_8005C650(4, 0x7F, 0x7F);
        }
    }
}
