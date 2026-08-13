/*
 * s16 REDERIVE / register-dial probe. The mirror of the above: block 1 written
 * as direct global accesses, the pointer live only across the bit-2 and bit-4
 * blocks. 14 at 49 insns, lui 458. The pointer still wins $a0, so restricting
 * its live range does not free $v1 either.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    {
        s32 v;
        s32 c;

        c = p[8] & 1;
        v = D_80106A73;
        if (c) {
            c = v | 1;
        } else {
            c = v;
        }
        D_80106A73 = c;
    }
    q = &D_80106A73;

    {
        s32 v;
        s32 c;

        q = &D_80106A73;
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

        q = &D_80106A73;
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
