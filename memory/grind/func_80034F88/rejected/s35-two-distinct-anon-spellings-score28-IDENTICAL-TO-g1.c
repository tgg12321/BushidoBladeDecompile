/* s35 (synthesis, 2026-09-05): as g1 but blocks 2 and 3 spelled with two DIFFERENT symbol-difference nestings, to test whether cse can be stopped from canonicalising them to the same address rtx. score 28 at 48 -- byte-identical outcome to g1, so cse canonicalises both spellings and 'distinct anonymous spellings' is not a lever. */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    q = &D_80106A73;
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
        v = (*(u8 *)((s32)&D_80106A73 + ((s32)&D_80106A70 - (s32)&D_80106A70)));
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        (*(u8 *)((s32)&D_80106A73 + ((s32)&D_80106A70 - (s32)&D_80106A70))) = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
