/*
 * s16 REDERIVE. The single pointer SET EXACTLY ONCE (no `q = &D_80106A73;`
 * restatement before blocks 2 and 3): 23 at 47 insns, lui 454 vs the target's
 * 456. With one set, local-alloc's update_equiv_regs records reg_equiv_constant
 * and substitutes the symbol back into the remaining references, so TWO of the
 * three address materialisations disappear. The restatements in candidate.c are
 * load-bearing for keeping the pseudo in a hard register.
 */
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
        v = *q;
        if (c) {
            c = v | 2;
        } else {
            c = v;
        }
        *q = c;
    }

    {
        s32 v;
        s32 c;

        c = p[8] & 4;
        v = *q;
        if (c) {
            c = v | 4;
        } else {
            c = v;
        }
        *q = c;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
