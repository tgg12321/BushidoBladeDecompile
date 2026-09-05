void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 v;
    s32 c;

    p = func_80077D00();
    {
        u8 *q = &D_80106A70[3];
        s32 u;
        c = *q;
        c = c & 0xF8;
        *q = c;
        c = p[8] & 1;
        u = *q;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
    }
    {
        u8 *r = &D_80106A70[3];
        v = *r;
        c = p[8] & 2;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *r = c;

        r = &D_80106A70[3];
        v = *r;
        c = p[8] & 4;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *r = c;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
