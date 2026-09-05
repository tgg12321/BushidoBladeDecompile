/* s45: block-1's p[8] load hoisted between m's def and the mask. 13 at 49; .lreg still shows 74 'used 6 times across 9 insns' -- live length is invariant under source statement order. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;
    s32 f;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    f = p[8];
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = f & 1;
        if (c) {
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
