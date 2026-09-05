/* s40 (synthesis, 2026-09-05) DISPROVEN -- s40-roundtrip-inline-helper-blocks23-score30.c
 * Round-trip chassis; blocks 2/3 replaced by two expansions of a static inline helper that declares its own `u8 *q`. 30 at 49. Both expansions SHARE one store base ($a1, the same register the caller-q gets) and their reads fold to bare `lui $v1` + %lo -- inlining does not buy a second register-held address, and the block shape degrades to beqz+nop. Reproduces s37-inline-helper-ptr-blocks23-score30 on the new chassis.
 */
static inline void bb2_set_flag(s32 c, s32 bit) {
    u8 *q = &D_80106A73;
    s32 v;
    s32 r;

    v = *q;
    if (c) {
        r = v | bit;
    } else {
        r = v;
    }
    *q = r;
}

void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 m;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    m = *q;
    m = m & 0xF8;
    *q = m;

    {
        s32 c;

        c = p[8] & 1;
        if (c) {
            c = m | 1;
        } else {
            c = m;
        }
        *q = c;
    }

    bb2_set_flag(p[8] & 2, 2);
    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
