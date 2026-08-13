/*
 * KILLED (s15).  Control for the declaration-order lever: identical body to
 * s15/variants/t1.c (score 10) except the two pointer objects are declared BEFORE
 * the six value locals.  Scores 21 — the same 21 s14 banked as r19.  Declaration
 * order alone is worth 11 points on the two-object chassis because it decides
 * global.c allocno_compare's pri-7500 tie between the mask+bit-1 pointer allocno
 * and the three loaded-byte allocnos.
 */
void func_80034F88(void) {
    s32 *p;
    s32 i;
    u8 *q;
    u8 *q2;
    s32 va;
    s32 vb;
    s32 vc;
    s32 ca;
    s32 cb;
    s32 cc;

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
