/* REJECTED (s11) - Block 1's byte in its own local (the target does put block 1's byte and blocks 2/3's byte in different hard registers). On the register-death chassis the split costs an instruction: 50 insns / 34. Free but inert on the plain-mask chassis (9/49) and inert on the h1 chassis (13).
 * source: tmp/grind/func_80034F88/s11/variants/f1_maskc_byte_m_val.c
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    s32 m;
    s32 val;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    c = *ptr & 0xF8;
    *ptr = c;

    c = p[8] & 1;
    m = *ptr;
    if (c) {
        c = m | 1;
    } else {
        c = m;
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
