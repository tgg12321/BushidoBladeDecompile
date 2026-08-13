/*
 * KILLED (s15).  s2's 'stage the NEXT block's pointer before the CURRENT block's
 * store' lever, re-tried on the two-object + declaration-order chassis.  Score 20
 * at 49 insns with lbu 174 (two reloads lost).  Staging lengthens the first
 * pointer's live range past the store, which raises its allocno live_length but
 * also moves its SET before the cse flush, so the fresh materialisation degrades
 * to a copy and the next block's load is forwarded away.  The lever that helped
 * on s2's one-armed chassis is negative on the two-armed if/else chassis.
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
        q2 = &D_80106A73;
        *q2 = cb;
    }

    {
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
