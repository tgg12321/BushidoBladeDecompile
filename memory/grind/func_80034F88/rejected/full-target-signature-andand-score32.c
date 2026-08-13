/*
 * s9. The cheapest form ever measured that carries the target's EXACT access
 * signature - lbu 5 / sb 5 - together with an unconditional store: two `&&`
 * boundaries plus the floor's pointer-mask/symbol-read mismatch for block 1.
 * 32 at 54 insns, lui 5 (target 4; the extra one is the symbol-spelled
 * block-1 read).  Signature is NOT score: 32 vs the floor's 18.
 */
void func_80034F88(void) {
    s32 *p;
    u8 *ptr1;
    u8 *ptr2;
    u8 *ptr3;
    s32 val;
    u8 val2;
    s32 c;
    s32 i;

    p = func_80077D00();
    ptr1 = &D_80106A73;
    *ptr1 &= 0xF8;

    c = p[8] & 1;
    val = D_80106A73;
    val2 = val | 1;
    if (!c && ptr1) {
        val2 = val;
    }
    ptr2 = &D_80106A73;
    *ptr1 = val2;

    c = p[8] & 2;
    val = *ptr2;
    val2 = val | 2;
    if (!c && ptr2) {
        val2 = val;
    }
    *ptr2 = val2;
    ptr3 = &D_80106A73;

    c = p[8] & 4;
    val = *ptr3;
    val2 = val | 4;
    if (!c) {
        val2 = val;
    }
    *ptr3 = val2;


    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
