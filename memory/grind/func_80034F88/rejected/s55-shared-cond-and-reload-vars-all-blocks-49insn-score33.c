void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    s32 v;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q & 0xF8;
    *q = m;

    m = p[8];
    v = *q;
    m &= 1;
    if (m) {
        m = v | 1;
    } else {
        m = v;
    }
    *q = m;

    q = &D_80106A73;
    v = *q;
    m = p[8] & 2;
    if (m) {
        m = v | 2;
    } else {
        m = v;
    }
    *q = m;

    q = &D_80106A73;
    v = *q;
    m = p[8] & 4;
    if (m) {
        m = v | 4;
    } else {
        m = v;
    }
    *q = m;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
