/* s37 (2026-09-05) DISPROVEN -- s37-inline-helper-ptr-block3-only-score16.c
 * Minimal inline-helper dose: only block 3 goes through the helper, blocks 0/1/2 through the caller's q. 16 at 49 insns.
 */
static inline void bb2_set_flag(s32 c, s32 bit) {
    u8 *q = &D_80106A73;
    s32 v;

    v = *q;
    if (c) {
        c = v | bit;
    } else {
        c = v;
    }
    *q = c;
}

void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = (*q);
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        (*q) = c;
    }

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        v = (*q);
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        (*q) = c;
    }

    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
