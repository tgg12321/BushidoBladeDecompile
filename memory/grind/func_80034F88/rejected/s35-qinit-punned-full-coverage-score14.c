/* s35 (synthesis, 2026-09-05): q initialised and re-assigned THROUGH the symbol-difference expression at all three materialisation points (base coverage, q:0123). score 14 at 49 build insns -- 4 points WORSE than the plain `q = &D_80106A73;` base, so the punned initialiser is a net loss on the full-coverage chassis. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
    (*q) &= 0xF8;

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

    {
        s32 v;
        s32 c;

        q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
        c = p[8] & 2;
        v = (*q);
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        (*q) = c;
    }

    {
        s32 v;
        s32 c;

        q = (u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
        c = p[8] & 4;
        v = (*q);
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        (*q) = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
