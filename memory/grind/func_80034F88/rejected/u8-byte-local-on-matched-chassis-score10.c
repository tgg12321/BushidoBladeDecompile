void func_80034F88(void) {
    s32 *p;
    s32 c;
    s32 i;

    p = func_80077D00();
    {
        u8 *q = &D_80106A73;

        *q = *q & 0xF8;
    }

    {
        u8 *q = &D_80106A73;
        u8 v;

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
        u8 *q = &D_80106A73;
        u8 v;

        c = p[8] & 2;
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        u8 *q = &D_80106A73;
        u8 v;

        c = p[8] & 4;
        v = *q;
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
