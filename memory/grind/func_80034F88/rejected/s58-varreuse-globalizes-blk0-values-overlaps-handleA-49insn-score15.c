void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 raw;
    s32 mv;
    s32 v;
    s32 c;

    p = func_80077D00();

    {
        /* FAKE: handle A */
        u8 *t = &D_80106A73;

        raw = *t;
        mv = raw & 0xF8;
        *t = mv;
        /* FAKE: value-side cse2 invalidator */
        mv = raw;
        v = *t;
        c = p[8] & 1;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *t = c;
    }

    {
        /* FAKE: handle B */
        u8 *q = &D_80106A73;

        mv = *q;
        c = p[8] & 2;
        if (c) {
            c = mv | 2;
        } else {
            c = mv;
        }
        *q = c;

        q = &D_80106A73;
        raw = *q;
        c = p[8] & 4;
        if (c) {
            c = raw | 4;
        } else {
            c = raw;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
