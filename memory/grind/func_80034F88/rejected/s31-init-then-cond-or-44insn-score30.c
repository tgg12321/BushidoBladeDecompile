void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 c;

        c = *q;
        if (p[8] & 1) {
            c = c | 1;
        }
        *q = c;
    }

    {
        s32 c;

        q = &D_80106A73;
        c = *q;
        if (p[8] & 2) {
            c = c | 2;
        }
        *q = c;
    }

    {
        s32 c;

        q = &D_80106A73;
        c = *q;
        if (p[8] & 4) {
            c = c | 4;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
