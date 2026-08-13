/*
 * REJECTED (s10, synthesis) — from b1_block1_read_ptr2.c
 *
 * s10 wave B — block 1 reading through the OTHER pointer variable, so its read
 * cannot be satisfied from the mask's stored value (the two mems are
 * `(mem (reg ptr2))` and `(mem (reg ptr))`, which do not hash equal and are not
 * merged on this chassis).  It does produce block 1's reload (lbu 5), but the
 * second pointer must then be materialised before block 1 rather than before
 * block 2, which adds an instruction (50 against the target's 49) and moves the
 * whole base sequence: 27.  Storing through the second pointer instead is
 * 13/49 (b2); doing both is 23/51 (b3).
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
    val = *ptr2;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    ptr2 = &D_80106A73;
    *ptr = c;

    c = p[8] & 2;
    val = *ptr2;
    if (c) {
        c = val | 2;
    } else {
        c = val;
    }
    *ptr2 = c;
    ptr2 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr2;
    if (c) {
        c = val | 4;
    } else {
        c = val;
    }
    *ptr2 = c;

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
