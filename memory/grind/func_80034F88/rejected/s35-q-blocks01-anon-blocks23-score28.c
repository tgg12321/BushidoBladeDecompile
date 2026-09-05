/* s35 (synthesis, 2026-09-05): q covers blocks 0+1 only, blocks 2/3 through the anonymous symbol-difference expression. score 28 at 48 build insns. q takes $a1 and p takes $a0 (both wrong); the anonymous blocks emit lui+lbu / lui+sb = 4 insns each, the SAME count as the target's lui+addiu+lbu+sb, so the anonymous carrier is instruction-count-neutral in blocks 2/3 -- the single missing instruction is block 1's reload, not a label re-materialisation. */
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
