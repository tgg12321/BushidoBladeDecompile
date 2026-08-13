/*
 * s16 REDERIVE. Same as m2c-natural-value-shape-score21.c with s32 rather than
 * u8 temporaries -- identical 21 / 45 insns, so the temporary's width is inert
 * on this shape and the fold is in the duplicate-read collapse, not the type.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;
    {
        s32 v;

        v = *q | 1;
        if (!(p[8] & 1)) {
            v = *q;
        }
        *q = v;
    }
    q = &D_80106A73;
    {
        s32 v;

        v = *q | 2;
        if (!(p[8] & 2)) {
            v = *q;
        }
        *q = v;
    }
    q = &D_80106A73;
    {
        s32 v;

        v = *q | 4;
        if (!(p[8] & 4)) {
            v = *q;
        }
        *q = v;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
