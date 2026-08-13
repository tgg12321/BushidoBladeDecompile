void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    u8 *q2;
    u8 *q3;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q2 = &D_80106A73;
        c = p[8] & 2;
        v = *q2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q2 = c;
    }

    {
        s32 v;
        s32 c;

        q3 = &D_80106A73;
        c = p[8] & 4;
        v = *q3;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q3 = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
