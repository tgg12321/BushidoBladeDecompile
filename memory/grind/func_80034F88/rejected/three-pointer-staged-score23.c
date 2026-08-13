/*
 * KILLED (s15).  Three pointer objects with each next block's handle staged
 * before the current block's store; 23 at 49 insns, lbu 175.  Same mechanism as
 * two-pointer-staged-before-store-score20.c: staging places the SET before the
 * cse value-table flush.  The three-object shape only reaches 0 when each handle
 * is set INSIDE its own block, after the previous join label (see
 * memory/grind/func_80034F88/candidate.c).
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
    u8 *q3;

    p = func_80077D00();
    q = &D_80106A73;
    *q &= 0xF8;

    {
        ca = p[8] & 1;
        va = *q;
        if (ca) {
            ca = va | 1;
        } else {
            ca = va;
        }
        q2 = &D_80106A73;
        *q = ca;
    }

    {
        cb = p[8] & 2;
        vb = *q2;
        if (cb) {
            cb = vb | 2;
        } else {
            cb = vb;
        }
        q3 = &D_80106A73;
        *q2 = cb;
    }

    {
        cc = p[8] & 4;
        vc = *q3;
        if (cc) {
            cc = vc | 4;
        } else {
            cc = vc;
        }
        *q3 = cc;
    }

    for (i = 0; i < 3; i++) {
        *(&D_80106A70 + i) = *((u8 *)p + i + 0x17);
    }
}
