void func_800324D0(u8 *a0) {
    register u8 *v1 asm("v1");
    register u8 v0 asm("v0");
    register u32 a2 asm("a2");
    register u8 a1 asm("a1");

    v1 = *(u8 **)(a0 + 0x58);
    v0 = 0xFF;
    a0[0xA1] = v0;
    a0[0xA3] = v0;
    a0[0xA2] = v0;
    a0[0xA4] = v0;
    a0[0xAA] = 0;
    a0[0xA7] = 0;
    a0[0xA8] = 0;
    a0[0xA5] = 0;
    a0[0xA6] = v0;
    a0[0xAB] = v0;
    a0[0xAC] = v0;

    v0 = v1[4];
    v1 += 5;
    if (v0 == 0) return;

    do {
        a2 = v0;
        if (a2 == 0xFF) {
            v1 += 6;
        } else if (a2 < 0x80) {
            v1++;
        } else {
            a2 -= 0x80;
            a1 = *v1;
            v1++;
            if (a2 < 12) {
                switch (a2) {
                    case 0: a0[0xA1] = a1; break;
                    case 1: a0[0xA3] = a1; break;
                    case 2: a0[0xA7] = a1; break;
                    case 3: a0[0xA8] = a1; break;
                    case 4: a0[0xA9] = a1; break;
                    case 5: a0[0xA5] = a1; break;
                    case 6: a0[0xA6] = a1; break;
                    case 7: a0[0xA2] = a1; break;
                    case 8: a0[0xA4] = a1; break;
                    case 9: a0[0xAA] = a1; break;
                    case 10: a0[0xAB] = a1; break;
                    case 11: a0[0xAC] = a1; break;
                }
            }
        }
        v0 = *v1;
        v1++;
    } while (v0 != 0);
}
