void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    volatile u8 *q;
    volatile u8 *q2;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;

    {
        u8 v;
        u8 r;
        s32 c;

        v = *q;
        c = p[8] & 1;
        if (c) {
            r = v | 1;
        } else {
            r = v;
        }
        *q = r;
    }

    {
        u8 v;
        u8 r;
        s32 c;

        q2 = &D_80106A73;
        v = *q2;
        c = p[8] & 2;
        if (c) {
            r = v | 2;
        } else {
            r = v;
        }
        *q2 = r;
    }

    {
        u8 v;
        u8 r;
        s32 c;

        q2 = &D_80106A73;
        v = *q2;
        c = p[8] & 4;
        if (c) {
            r = v | 4;
        } else {
            r = v;
        }
        *q2 = r;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
