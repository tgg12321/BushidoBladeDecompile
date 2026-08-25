/* s16 DIAGNOSTIC (score 9): target's source order (sum=0 first) but STILL two
 * separate counters.  ra_solver: sum(77) pri 27272 (n=10, LL=11), j(79) pri 47142
 * (n=11, LL=7) -> 3-way rotation bp=$a0 / sum=$a1 / j=$v1.  This is the chassis on
 * which the inverse solver returned the single-atom vector `live_extend j 7->15`,
 * i.e. merge j with the Region-B counter -- which candidate.c does. */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;
    s32 offset;

    i = 0;
    chkptr = (s32 *)base;
    offset = 0;
    do {
        s32 sum;
        u8 *bp;
        u32 j;

        sum = 0;
        bp = base + offset;
        j = 0;
        do {
            sum += *bp;
            bp++;
            j++;
        } while (j < 0x24U);
        if (sum == *(s32 *)((u8 *)chkptr + 0x6C)) {
            break;
        }
        chkptr++;
        i++;
        offset += 0x24;
    } while (i < 3);

    if (i == 3) {
        return 0;
    }

    if (D_800A31FC != 0) {
        return 1;
    }

    {
        u8 *src = base + i * 0x24;
        s32 k;

        if (!(*(src + 0x23) & 0x80)) {
            CopyBlock *dst = (CopyBlock *)&D_80106A50;
            CopyBlock *sp2 = (CopyBlock *)src;
            CopyBlock *end = (CopyBlock *)((u8 *)src + 0x20);
            for (;;) {
                *dst = *sp2;
                sp2++;
                dst++;
                if (sp2 != end) continue;
                *(s32 *)dst = *(s32 *)sp2;
                break;
            }
        }

        do { k = 0; } while (0); /* FAKE: do-while(0) RA weighting + biv-init fold (Region B) */
        do {
            u16 *ptr = *(u16 **)(base + k * 4 + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(base + k * 2 + 0xD0);
            }
            k++;
        } while (k < 0x16);
    }

    return 1;
}
