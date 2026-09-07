void func_8007526C(void) {
    u8 *base;
    u8 *p;
    s32 i;
    s32 lim;

    base = D_800A36A0;
    i = 0;
    do {
        lim = 0xC8;
        { long long d0 = 0x100000000LL + 0; }
        { long long d1 = 0x100000000LL + 1; }
        { long long d2 = 0x100000000LL + 2; }
        { long long d3 = 0x100000000LL + 3; }
        { long long d4 = 0x100000000LL + 4; }
        { long long d5 = 0x100000000LL + 5; }
        { long long d6 = 0x100000000LL + 6; }
        { long long d7 = 0x100000000LL + 7; }
        { long long d8 = 0x100000000LL + 8; }
        { long long d9 = 0x100000000LL + 9; }
        { long long d10 = 0x100000000LL + 10; }
        { long long d11 = 0x100000000LL + 11; }
        { long long d12 = 0x100000000LL + 12; }
        { long long d13 = 0x100000000LL + 13; }
        { long long d14 = 0x100000000LL + 14; }
        { long long d15 = 0x100000000LL + 15; }
        { long long d16 = 0x100000000LL + 16; }
        { long long d17 = 0x100000000LL + 17; }
        { long long d18 = 0x100000000LL + 18; }
        { long long d19 = 0x100000000LL + 19; }
        { long long d20 = 0x100000000LL + 20; }
        { long long d21 = 0x100000000LL + 21; }
        { long long d22 = 0x100000000LL + 22; }
        { long long d23 = 0x100000000LL + 23; }
        { long long d24 = 0x100000000LL + 24; }
        { long long d25 = 0x100000000LL + 25; }
        { long long d26 = 0x100000000LL + 26; }
        { long long d27 = 0x100000000LL + 27; }
        { long long d28 = 0x100000000LL + 28; }
        { long long d29 = 0x100000000LL + 29; }
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
