void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;

    p = func_80077D00();

    {
        /* FAKE: handle A */
        u8 *t = &D_80106A73;
        s32 v;
        s32 c;

        c = p[8];
        m = *t;
        m &= 0xF8;
        *t = m;

        m = c;
        v = *t;
        m &= 1;
        if (m) {
            m = v | 1;
        } else {
            m = v;
        }
        *t = m;
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

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
