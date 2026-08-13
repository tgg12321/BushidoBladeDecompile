/* REJECTED (s11) - h1's copy handle for block 1 plus a DEDICATED third pointer local for blocks 2/3, to keep that base out of the coalesced pair's register. 18 / 49 - the split costs more than the swap it was meant to fix (i2/i3/i6 18, i1 loses the reload back to 9).
 * source: tmp/grind/func_80034F88/s11/variants/i2_copy_rw.c
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
    *ptr2 = c;
    ptr3 = &D_80106A73;

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
