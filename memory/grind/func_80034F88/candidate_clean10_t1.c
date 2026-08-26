/*
 * s15 BEST UNAMBIGUOUSLY NON-BANNED FORM — honest sandbox score 10 at 49/49 insns
 * (lbu 175 / sb 164 / lui 456).  TWO function-scope pointer objects (`q` for the
 * mask + bit-1 segment, `q2` re-assigned for the bit-2 and bit-4 blocks) plus the
 * declaration-order lever: the six value locals are declared BEFORE the pointers,
 * which flips global.c allocno_compare's tie-break (`return *v1 - *v2;` on the
 * allocno number, which follows pseudo number, which follows declaration order)
 * when the pointer allocno and the byte allocnos tie at pri 7500.  Worth 11
 * points: the same body with the pointers declared first (s15/variants/t2.c) is
 * the 21 that s14 banked as r19.
 * This ties s14's one-pointer floor of 10 and has the identical residual — the
 * mask + bit-1 segment's base/byte pair is swapped against target (build base $a0
 * byte $v1, target base $v1 byte $a0) and the bit-1 reload is missing.  It is the
 * form the floor of 10 is recorded against.
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
