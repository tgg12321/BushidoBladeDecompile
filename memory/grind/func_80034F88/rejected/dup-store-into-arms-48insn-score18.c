/* REJECTED (s11) - The store duplicated into both if/else arms, reading the target's tail as a jump2 cross-jump merge. Emits 48 instructions against the target's 49 - the stores merge, the value copy does not - at 18. The label-shared spelling is the correct reading of that tail (c1-c6, c9, c10: 18-35).
 * source: tmp/grind/func_80034F88/s11/variants/c1_dup_ptr_zz5.c
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        *ptr = val | 1;
    } else {
        *ptr = val;
    }
    ptr2 = &D_80106A73;

    c = p[8] & 2;
    val = *ptr2;
    if (c) {
        *ptr2 = val | 2;
    } else {
        *ptr2 = val;
    }
    ptr2 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr2;
    if (c) {
        *ptr2 = val | 4;
    } else {
        *ptr2 = val;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
