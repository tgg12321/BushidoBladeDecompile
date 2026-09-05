/* s35 (synthesis, 2026-09-05): q initialised THROUGH the symbol-difference expression (so the cse copy preference exists at q's birth), q covers blocks 0+1, blocks 2/3 anonymous. score 27 at 48 build insns -- the c1 preference lever does not survive when q's own initialiser is the punned expression. */
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
