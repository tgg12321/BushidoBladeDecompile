void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 v;
    u8 r;
    s32 c;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    v = *q & 0xF8;
    *q = v;

    v = *q;
    c = p[8] & 1;
    if (c) {
        r = v | 1;
    } else {
        r = v;
    }
    *q = r;

    q = &D_80106A73;
    v = *q;
    c = p[8] & 2;
    if (c) {
        r = v | 2;
    } else {
        r = v;
    }
    *q = r;

    q = &D_80106A73;
    v = *q;
    c = p[8] & 4;
    if (c) {
        r = v | 4;
    } else {
        r = v;
    }
    *q = r;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
