void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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

        c = p[8] & 2;
        v = D_80106A73;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = D_80106A73;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
