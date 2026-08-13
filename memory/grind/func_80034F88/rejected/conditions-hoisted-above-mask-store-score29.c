void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    s32 c1;
    s32 c2;
    s32 c3;

    p = func_80077D00();
    c1 = p[8] & 1;
    c2 = p[8] & 2;
    c3 = p[8] & 4;
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 t;

        v = *q;
        if (c1) {
            t = v | 1;
        } else {
            t = v;
        }
        *q = t;
    }
    {
        s32 v;
        s32 t;

        q = &D_80106A73;
        v = *q;
        if (c2) {
            t = v | 2;
        } else {
            t = v;
        }
        *q = t;
    }
    {
        s32 v;
        s32 t;

        q = &D_80106A73;
        v = *q;
        if (c3) {
            t = v | 4;
        } else {
            t = v;
        }
        *q = t;
    }
    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
