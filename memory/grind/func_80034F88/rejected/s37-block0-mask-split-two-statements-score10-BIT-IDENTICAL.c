/* s37 (2026-09-05) DISPROVEN -- s37-block0-mask-split-two-statements-score10-BIT-IDENTICAL.c
 * Block 0's mask split into `m = *q; m = m & 0xF8; *q = m;` to stretch the value pseudo. 10 at 49, objdump bit-identical to the base body's.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
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
