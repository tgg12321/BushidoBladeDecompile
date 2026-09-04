void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    s32 v;
    s32 c;

    p = func_80077D00();
    q = &D_80106A73;
    v = *q & 0xF8;
    *q = v;

    c = p[8] & 1;
    if (c) {
        c = v | 1;
    } else {
        c = v;
    }
    *q = c;

    q = &D_80106A73;
    v = c;
    c = p[8] & 2;
    if (c) {
        c = v | 2;
    } else {
        c = v;
    }
    *q = c;

    q = &D_80106A73;
    c = p[8] & 4;
    v = *q;
    if (c) {
        c = v | 4;
    } else {
        c = v;
    }
    *q = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
