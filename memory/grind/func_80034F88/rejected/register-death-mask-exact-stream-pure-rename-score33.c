/* REJECTED (s11) - s10's register-death reload mechanism on the two-pointer base structure. Emits the target's COMPLETE 49-instruction stream - same opcodes, same order, same delay slots, no nop - with a pure hard-register permutation (build base1 a2 / p a3 / base2 a1 / byte v1 / cond v1 against target v1 / a1 / a0 / a0 / v0). 33. Superseded by h1, which reaches the same stream with the correct registers through block 1.
 * source: tmp/grind/func_80034F88/s11/variants/b2_maskc_zz5.c
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
    c = *ptr & 0xF8;
    *ptr = c;

    c = p[8] & 1;
    val = *ptr;
    if (c) {
        c = val | 1;
    } else {
        c = val;
    }
    *ptr = c;
    ptr2 = &D_80106A73;

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
