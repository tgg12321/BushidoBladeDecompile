void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m &= 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            *q = m | 1;
        } else {
            *q = m;
        }
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        c = p[8] & 1;
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
        c = p[8] & 2;
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
