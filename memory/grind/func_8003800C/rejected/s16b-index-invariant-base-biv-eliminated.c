/* s16b KILLED as a match (score 4 -- but the BEST FAKE-free score ever recorded
 * on this function, vs 9 for the two-counter sum-first chassis and 4 for the
 * plain j-first one).
 * PROBE B -- the checksum accumulate written as an INDEX over a loop-invariant
 * base:  p = base + offset;  do { sum += p[j]; j++; } while (j < 0x24U);
 * (s10's rejected regionA-index-accumulate-diverges.c indexed base[offset + j],
 *  which has TWO varying indices and cannot be strength-reduced; this form's
 *  base is invariant for the inner loop, so loop.c reduces it cleanly.)
 * RESULT: loop.c strength-reduces p[j] to a walking giv AND THEN ELIMINATES the
 * biv j entirely (all of j's remaining uses are the loop compare, which becomes
 * a giv-vs-giv pointer compare).  The 4 residual insns ARE the counter:
 *   ours:  addiu $a1,$v1,0x24 (end ptr) / addiu $v1,$v1,1 / sltu $v0,$v1,$a1 / nop
 *   tgt :  addu  $a1,$zero,$zero (j=0)  / addiu $a1,$a1,1 / sltiu $v0,$a1,0x24
 *          + addiu $v1,$v1,1 in the branch delay slot
 * IMPORTANT POSITIVE RESULT: with j gone as a competing allocno the register
 * seats become EXACTLY target's -- sum(77) takes $a0 (pri 30000), the walking
 * giv(124) takes $v1 (pri 91428), the end-pointer giv(125) takes $a1.  So the
 * target disposition is reachable with ZERO coercion the moment j stops being
 * an allocno; the entire Region-A residual is the cost of keeping j alive, which
 * target's bytes require.  This is the cleanest statement yet of the lock.
 */
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
        u8 *p;
        u32 j;

        sum = 0;
        p = base + offset;
        j = 0;
        do {
            sum += p[j];
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
