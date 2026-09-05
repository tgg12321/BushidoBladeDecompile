/* s42 (synthesis, 2026-09-05): value chaining over blocks 0-2 only, block 3
 * re-reading the flag byte.  24 at 47 insns -- two lost reloads instead of
 * four.  Same defect as the all-blocks chain, scaled down.
 * DO NOT RE-PROPOSE. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    s32 v;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            v = m | 1;
        } else {
            v = m;
        }
        *q = v;
    }

    {
        s32 c;

        q = &D_80106A73;
        c = p[8] & 2;
        if (c) {
            m = v | 2;
        } else {
            m = v;
        }
        *q = m;
    }

    {
        s32 c;
        s32 w;

        q = &D_80106A73;
        c = p[8] & 4;
        w = *q;
        if (c) {
            c = w | 4;
        } else {
            c = w;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
