/* s40 (synthesis, 2026-09-05) DISPROVEN -- s40-roundtrip-q-blocks01-directsymbol23-score28.c
 * As above but blocks 2/3 spelled as the bare symbol (`v = D_80106A73; ... D_80106A73 = c;`). 28 at 48 insns -- byte-for-byte the same stream as the symbol-difference spelling, confirming the symdiff expression constant-folds to the plain symbol_ref before RTL address selection.
 */
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

    {
        s32 v;
        s32 c;

        c = p[8] & 2;
        v = D_80106A73;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = D_80106A73;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
