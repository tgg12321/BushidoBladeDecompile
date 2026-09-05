void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 v;
    s32 c;
    s32 c1;
    s32 c2;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, mechanism: global.c:1275 assigns
         * exactly one hard register per allocno and GCC 2.7.2 does no
         * live-range splitting. lever-exhaustion: hypotheses.md s53-s60. */
        u8 *q = &D_80106A73;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = 0; /* FAKE: cse2 value invalidator, mechanism: cse2 (cse.c) forwards
                * the sb into the following lbu only while the stored value's
                * pseudo still holds it. lever-exhaustion: hypotheses.md s57-s62. */
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
    }
    {
        /* FAKE: the address object for flag blocks 1 and 2, mechanism: as
         * above. lever-exhaustion: as above. */
        u8 *r = &D_80106A73;

        v = *r;
        c1 = p[8] & 2;
        if (c1) {
            c1 = v | 2;
        } else {
            c1 = v;
        }
        *r = c1;

        r = &D_80106A73;
        v = *r;
        c2 = p[8] & 4;
        if (c2) {
            c2 = v | 4;
        } else {
            c2 = v;
        }
        *r = c2;
    }

    for (i = 0; i < 3; i++) {
        D_80106A70[i] = *((u8 *)p + i + 0x17);
    }
}
