/* s37 (2026-09-05) DISPROVEN -- s37-inline-helper-ptr-blocks123-reuse-shape-score13.c
 * Best inline-helper body measured: helper mirrors candidate.c's block shape exactly (the condition parameter is reused as the merged value), inlined for blocks 1/2/3. 13 at 50 insns -- three points above the base floor and one instruction long.
 */
static inline void bb2_set_flag(s32 c, s32 bit) {
    u8 *q = &D_80106A73;
    s32 v;

    v = *q;
    if (c) {
        c = v | bit;
    } else {
        c = v;
    }
    *q = c;
}

void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    bb2_set_flag(p[8] & 1, 1);

    bb2_set_flag(p[8] & 2, 2);

    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
