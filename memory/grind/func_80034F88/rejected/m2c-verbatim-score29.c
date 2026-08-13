/*
 * s9. m2c's own re-derivation, verbatim (three per-block u8 selects, no
 * staging local, two symbol reads per block, m2c's do-while copy loop).
 * 29 at 45 insns, lbu 2 / lui 6.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *q;
    u8 v1;
    u8 v2;
    u8 v3;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    v1 = D_80106A73 | 1;
    if (!(p[8] & 1)) {
        v1 = D_80106A73;
    }
    D_80106A73 = v1;

    v2 = D_80106A73 | 2;
    if (!(p[8] & 2)) {
        v2 = D_80106A73;
    }
    D_80106A73 = v2;

    v3 = D_80106A73 | 4;
    if (!(p[8] & 4)) {
        v3 = D_80106A73;
    }
    D_80106A73 = v3;


    i = 0;
    q = (u8 *)p;
    do {
        *(&D_80106A70 + i) = q[0x17];
        i += 1;
        q = (u8 *)p + i;
    } while (i < 3);
}
