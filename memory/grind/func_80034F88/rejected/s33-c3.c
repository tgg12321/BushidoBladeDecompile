/* s33 -- q used in block 1 only, blocks 0/2/3 anonymous: score 14 at 48 build insns.
 * Blocks 2 and 3 lose a materialisation to each other and the body falls one insn
 * short of the target's 49.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) &= 0xF8;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
        c = p[8] & 1;
        v = *q;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 2;
        v = *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70));
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *(u8 *)((s32)&D_80106A70 + ((s32)&D_80106A73 - (s32)&D_80106A70)) = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
