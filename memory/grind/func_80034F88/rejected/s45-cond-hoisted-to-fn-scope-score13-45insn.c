/* s45: block-1 condition hoisted to function scope to stretch pseudo 74's live range. 13 at 45 -- the body collapses by four instructions; .lreg unchanged for 74. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;
    s32 c0;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    c0 = p[8] & 1;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        if (c0) {
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
