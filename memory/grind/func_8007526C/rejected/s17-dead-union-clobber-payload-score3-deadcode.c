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
        { union un q0 = { 1000 }; }
        { union un q1 = { 1001 }; }
        { union un q2 = { 1002 }; }
        { union un q3 = { 1003 }; }
        { union un q4 = { 1004 }; }
        { union un q5 = { 1005 }; }
        { union un q6 = { 1006 }; }
        { union un q7 = { 1007 }; }
        { union un q8 = { 1008 }; }
        { union un q9 = { 1009 }; }
        { union un q10 = { 1010 }; }
        { union un q11 = { 1011 }; }
        { union un q12 = { 1012 }; }
        { union un q13 = { 1013 }; }
        { union un q14 = { 1014 }; }
        { union un q15 = { 1015 }; }
        { union un q16 = { 1016 }; }
        { union un q17 = { 1017 }; }
        { union un q18 = { 1018 }; }
        { union un q19 = { 1019 }; }
        { union un q20 = { 1020 }; }
        { union un q21 = { 1021 }; }
        { union un q22 = { 1022 }; }
        { union un q23 = { 1023 }; }
        { union un q24 = { 1024 }; }
        { union un q25 = { 1025 }; }
        { union un q26 = { 1026 }; }
        { union un q27 = { 1027 }; }
        { union un q28 = { 1028 }; }
        { union un q29 = { 1029 }; }
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
