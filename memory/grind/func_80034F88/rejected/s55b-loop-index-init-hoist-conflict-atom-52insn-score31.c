void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    i = 0;

    {
        /* FAKE: handle A */
        u8 *t = &D_80106A73;
        u8 m;
        s32 c;

        m = *t;
        m &= 0xF8;
        *t = m;

        c = p[8];
        m = c;
        m = *t;
        c &= 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
        }
        *t = c;
    }

    {
        /* FAKE: handle B */
        u8 *q = &D_80106A73;
        s32 v;
        s32 c;

        v = *q;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
