/* REJECTED (s11) - Blocks 2/3's base assigned between block 1's if/else and its store, so its live range overlaps base 1's and cannot inherit v1. 18 / 49. Every placement variant of the same idea (k2 13, k3 15, k4 26, k5 23/50) is at or above h1's 13: the base/byte register swap is an allocno-priority tie that no placement reaches.
 * source: tmp/grind/func_80034F88/s11/variants/k1_base2_early.c
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    u8 *ptr3;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    val = *ptr & 0xF8;
    *ptr = val;
    ptr2 = ptr;

    c = p[8] & 1;
    val = *ptr2;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    ptr3 = &D_80106A73;
    *ptr2 = c;

    c = p[8] & 2;
    val = *ptr3;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr3 = c;
    ptr3 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr3;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr3 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
