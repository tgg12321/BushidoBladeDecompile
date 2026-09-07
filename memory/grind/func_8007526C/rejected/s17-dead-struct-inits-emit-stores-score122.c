void func_8007526C(void) {
    struct pr { s16 a; s16 b; };
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        { struct pr q0 = { 0, 1 }; }
        { struct pr q1 = { 1, 2 }; }
        { struct pr q2 = { 2, 3 }; }
        { struct pr q3 = { 3, 4 }; }
        { struct pr q4 = { 4, 5 }; }
        { struct pr q5 = { 5, 6 }; }
        { struct pr q6 = { 6, 7 }; }
        { struct pr q7 = { 7, 8 }; }
        { struct pr q8 = { 8, 9 }; }
        { struct pr q9 = { 9, 10 }; }
        { struct pr q10 = { 10, 11 }; }
        { struct pr q11 = { 11, 12 }; }
        { struct pr q12 = { 12, 13 }; }
        { struct pr q13 = { 13, 14 }; }
        { struct pr q14 = { 14, 15 }; }
        { struct pr q15 = { 15, 16 }; }
        { struct pr q16 = { 16, 17 }; }
        { struct pr q17 = { 17, 18 }; }
        { struct pr q18 = { 18, 19 }; }
        { struct pr q19 = { 19, 20 }; }
        { struct pr q20 = { 20, 21 }; }
        { struct pr q21 = { 21, 22 }; }
        { struct pr q22 = { 22, 23 }; }
        { struct pr q23 = { 23, 24 }; }
        { struct pr q24 = { 24, 25 }; }
        { struct pr q25 = { 25, 26 }; }
        { struct pr q26 = { 26, 27 }; }
        { struct pr q27 = { 27, 28 }; }
        { struct pr q28 = { 28, 29 }; }
        { struct pr q29 = { 29, 30 }; }
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
