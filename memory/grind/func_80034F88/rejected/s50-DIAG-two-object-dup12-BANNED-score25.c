void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *t;
    u8 *q;

    p = func_80077D00();
    t = &D_80106A73;
    m = *t;
    m &= 0xF8;
    *t = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
        }
        *t = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 2;
        if (c) {
            *q = v | 2;
        } else {
            *q = v;
        }
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 4;
        if (c) {
            *q = v | 4;
        } else {
            *q = v;
        }
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
