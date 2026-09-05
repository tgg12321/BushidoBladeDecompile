/* s39: v2 chassis with the whole p[8] word hoisted into c0 before block 0's mask and the three conditions taken from it. 27 at 48. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    s32 c0;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    c0 = p[8];
    m = m & 0xF8;
    *q = m;
    if (c0 & 1) {
        c0 = m | 1;
    } else {
        c0 = m;
    }
    *q = c0;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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
