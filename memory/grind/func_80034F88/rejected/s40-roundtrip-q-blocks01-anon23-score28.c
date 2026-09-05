/* s40 (synthesis, 2026-09-05) DISPROVEN -- s40-roundtrip-q-blocks01-anon23-score28.c
 * Round-trip chassis (candidate.c block 0) with q covering blocks 0/1 only and blocks 2/3 through the anonymous symbol-difference expression. 28 at 48 insns. q takes $a1, p takes $a0. DECISIVE: the anonymous blocks emit `lui $at/$v0` + %lo-folded lbu/sb -- the compiler NEVER materialises the address with lui+addiu into an allocatable register unless the C source assigns &D_80106A73 to a pointer OBJECT. The target blocks 2/3 use lui+addiu+lbu+sb through one register. See evidence.md "==== s40 ====".
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
        v = (*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)));
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        (*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))) = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = (*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)));
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        (*(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70))) = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
