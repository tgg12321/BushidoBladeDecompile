/* s37 (2026-09-05) DISPROVEN -- s37-inline-helper-symbol-no-pointer-anywhere-score30.c
 * static inline helper accessing the plain symbol (ZERO pointer objects in the whole function). 30 at 44 insns -- five instructions short of the target.
 */
static inline void bb2_set_flag(s32 c, s32 bit) {
    s32 v;
    s32 r;

    v = D_80106A73;
    if (c) {
        r = v | bit;
    } else {
        r = v;
    }
    D_80106A73 = r;
}

void func_80034F88(void) {
    s32 *p;
    s32 i;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    bb2_set_flag(p[8] & 1, 1);

    bb2_set_flag(p[8] & 2, 2);

    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
