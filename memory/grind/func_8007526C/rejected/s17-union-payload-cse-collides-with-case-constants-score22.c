void func_8007526C(void) {
    union un { s32 w; };
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        { union un q0 = { 0 }; }
        { union un q1 = { 1 }; }
        { union un q2 = { 2 }; }
        { union un q3 = { 3 }; }
        { union un q4 = { 4 }; }
        { union un q5 = { 5 }; }
        { union un q6 = { 6 }; }
        { union un q7 = { 7 }; }
        { union un q8 = { 8 }; }
        { union un q9 = { 9 }; }
        { union un q10 = { 10 }; }
        { union un q11 = { 11 }; }
        { union un q12 = { 12 }; }
        { union un q13 = { 13 }; }
        { union un q14 = { 14 }; }
        { union un q15 = { 15 }; }
        { union un q16 = { 16 }; }
        { union un q17 = { 17 }; }
        { union un q18 = { 18 }; }
        { union un q19 = { 19 }; }
        { union un q20 = { 20 }; }
        { union un q21 = { 21 }; }
        { union un q22 = { 22 }; }
        { union un q23 = { 23 }; }
        { union un q24 = { 24 }; }
        { union un q25 = { 25 }; }
        { union un q26 = { 26 }; }
        { union un q27 = { 27 }; }
        { union un q28 = { 28 }; }
        { union un q29 = { 29 }; }
        p = base + i * 2;
        switch (*(u8 *)(p + 0x10)) {
        case 1:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) + 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
            }
            break;
        case 3:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) + 0xA;
            if ((s16)*(u16 *)(p + 0xC) >= 0xC8) {
                *(u16 *)(p + 8) = lim;
                *(u16 *)(p + 0xC) = lim;
                *(u16 *)(p + 0x14) = *(u16 *)(p + 0x18);
                *(u16 *)(p + 0x3C) = *(u16 *)(p + 0x38);
                if ((*(u16 *)(p + 0x10) >> 8) == 0) {
                    *(u16 *)(p + 0x10) = *(u16 *)(p + 0x10) + 1;
                }
            }
            break;
        case 2:
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        case 4:
            *(u16 *)(p + 8) = *(u16 *)(p + 8) - 0xA;
            *(u16 *)(p + 0xC) = *(u16 *)(p + 0xC) - 0xA;
            if ((s16)*(u16 *)(p + 0xC) <= 0) {
                *(u16 *)(p + 8) = 0;
                *(u16 *)(p + 0xC) = 0;
                *(u16 *)(p + 0x10) = 0;
            }
            break;
        }
        i++;
    } while (i < 2);
}
