/*
 * s9. Same mismatch with the store through the pointer: SEVEN lbu (every read
 * survives) at 48 insns, lui 5.  24 - the best of the mismatch family, still
 * 6 over the floor.
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
        val2 = *ptr;
    }
    *ptr = val2;

    c = p[8] & 2;
    val2 = D_80106A73 | 2;
    if (!c) {
        val2 = *ptr;
    }
    *ptr = val2;

    c = p[8] & 4;
    val2 = D_80106A73 | 4;
    if (!c) {
        val2 = *ptr;
    }
    *ptr = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
