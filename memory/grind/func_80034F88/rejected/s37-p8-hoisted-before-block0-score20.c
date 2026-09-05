/* s37 (2026-09-05) DISPROVEN -- s37-p8-hoisted-before-block0-score20.c
 * p[8] hoisted into a local before block 0 (F2 probe: give the scheduler no lw to shadow in block 1's load-delay slot). 20 at 47 -- two instructions short.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 f;
    u8 *q;

    p = func_80077D00();
    f = p[8];
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = f & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = f & 2;
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
        c = f & 4;
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
