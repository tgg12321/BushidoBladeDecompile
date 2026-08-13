/*
 * s9. m2c's two-reads-per-block select with the floor's pointer mask and
 * condition staging: 20 at 49 insns - the best NON-floor chassis measured
 * this session, and 2 worse than the floor's 18.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val2 = D_80106A73 | 1;
    if (!c) {
        val2 = D_80106A73;
    }
    D_80106A73 = val2;

    c = p[8] & 2;
    val2 = D_80106A73 | 2;
    if (!c) {
        val2 = D_80106A73;
    }
    D_80106A73 = val2;

    c = p[8] & 4;
    val2 = D_80106A73 | 4;
    if (!c) {
        val2 = D_80106A73;
    }
    D_80106A73 = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
