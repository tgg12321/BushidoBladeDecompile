/*
 * KILLED (s15).  The 0xF8 mask done through the plain symbol (`D_80106A73 &=
 * 0xF8;`) with two pointer objects for the flag blocks.  Score 16 at 51 insns,
 * lbu 176 / lui 458.  This is the first two-object form to produce ALL FOUR
 * target reloads, but the symbol-spelled mask pays its own address
 * materialisation instead of sharing the bit-1 block's base, so it is 2
 * instructions and 2 luis long.  Target's mask block and bit-1 block share one
 * unfolded $v1 base.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    s32 va;
    s32 vb;
    s32 vc;
    s32 ca;
    s32 cb;
    s32 cc;
    u8 *q;
    u8 *q2;

    p = func_80077D00();
    D_80106A73 &= 0xF8;

    {
        q = &D_80106A73;
        ca = p[8] & 1;
        va = *q;
        if (ca) {
            ca = va | 1;
        } else {
            ca = va;
        }
        *q = ca;
    }

    {
        q2 = &D_80106A73;
        cb = p[8] & 2;
        vb = *q2;
        if (cb) {
            cb = vb | 2;
        } else {
            cb = vb;
        }
        *q2 = cb;
    }

    {
        q2 = &D_80106A73;
        cc = p[8] & 4;
        vc = *q2;
        if (cc) {
            cc = vc | 4;
        } else {
            cc = vc;
        }
        *q2 = cc;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
