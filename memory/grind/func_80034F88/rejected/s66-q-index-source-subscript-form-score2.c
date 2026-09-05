void func_80034F88(void) {
    s32 *p;
    s32 v;
    s32 c;
    s32 u;

    p = func_80077D00();
    {
        /* FAKE: block-0's own address object, mechanism: global.c:1275 assigns
         * exactly one hard register per allocno. lever-exhaustion: hypotheses.md s53-s65. */
        u8 *q = &D_80106A73;

        u = *q;
        u = u & 0xF8;
        *q = u;
        u = 0;
        u = *q;
        c = p[8] & 1;
        if (c) {
            c = u | 1;
        } else {
            c = u;
        }
        *q = c;
        {
            /* FAKE: the address object for flag blocks 1 and 2, mechanism: as above. */
            u8 *r = &D_80106A73;

            v = *r;
            c = p[8] & 2;
            if (c) {
                c = v | 2;
            } else {
                c = v;
            }
            *r = c;

            r = &D_80106A73;
            v = *r;
            c = p[8] & 4;
            if (c) {
                c = v | 4;
            } else {
                c = v;
            }
            *r = c;
        }

        for (q = 0; (s32)q < 3; q++) {
            c = ((u8 *)p)[(s32)q + 0x17];
            D_80106A70[(s32)q] = c;
        }
    }
}
