/* REJECTED (s11) - Type narrowing of the value locals on the 9-point base structure. u8 byte adds masking instructions (51 insns); nine type crossings measured 18-33, none reaching the 9. Signed-char is worse still (58 insns / 23).
 * source: tmp/grind/func_80034F88/s11/variants/g_u8val_u8c_plain.c
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr;
    u8 *ptr2;
    u8 val;
    u8 c;
    s32 i;

    p = func_80077D00();
    ptr = &D_80106A73;
    *ptr &= 0xF8;

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
