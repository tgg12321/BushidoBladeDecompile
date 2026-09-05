/* s45: block-1 and block-2 conditions both hoisted. 25 at 40. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;
    s32 c1;
    s32 c2;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    c1 = p[8] & 1;
    c2 = p[8] & 2;
    *q = m;

    {
        s32 c;

        if (c1) {
            c = m | 1;
        } else {
            c = m;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        v = *q;
        if (c2) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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
