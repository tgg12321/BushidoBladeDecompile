void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;
    union qb { u8 c; };

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        { union qb q0 = { 100 }; }
        { union qb q1 = { 101 }; }
        { union qb q2 = { 102 }; }
        { union qb q3 = { 103 }; }
        { union qb q4 = { 104 }; }
        { union qb q5 = { 105 }; }
        { union qb q6 = { 106 }; }
        { union qb q7 = { 107 }; }
        { union qb q8 = { 108 }; }
        { union qb q9 = { 109 }; }
        { union qb q10 = { 110 }; }
        { union qb q11 = { 111 }; }
        { union qb q12 = { 112 }; }
        { union qb q13 = { 113 }; }
        { union qb q14 = { 114 }; }
        { union qb q15 = { 115 }; }
        { union qb q16 = { 116 }; }
        { union qb q17 = { 117 }; }
        { union qb q18 = { 118 }; }
        { union qb q19 = { 119 }; }
        { union qb q20 = { 100 }; }
        { union qb q21 = { 101 }; }
        { union qb q22 = { 102 }; }
        { union qb q23 = { 103 }; }
        { union qb q24 = { 104 }; }
        { union qb q25 = { 105 }; }
        { union qb q26 = { 106 }; }
        { union qb q27 = { 107 }; }
        { union qb q28 = { 108 }; }
        { union qb q29 = { 109 }; }
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
