/* s37 (2026-09-05) DISPROVEN -- s37-inline-helper-ptr-blocks23-score30.c
 * static inline helper (its own `u8 *q`) inlined for blocks 2/3; blocks 0/1 through the caller's q. 30 at 49 insns. Inlining DOES fire (no call in the .o) and DOES create a second address pseudo, but the two inlined copies share ONE store base in $a1 while their reads fold to bare lui+%lo -- the target's reused-full-address read base is lost, and p is displaced from $a1 to $a0.
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
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = (*q);
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        (*q) = c;
    }

    bb2_set_flag(p[8] & 2, 2);

    bb2_set_flag(p[8] & 4, 4);

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
