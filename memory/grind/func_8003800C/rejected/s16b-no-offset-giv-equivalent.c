/* s16b KILLED (score 9, byte-identical result to the explicit-offset spelling).
 * PROBE D -- drop the `offset` local and write the preheader as
 * `bp = base + i * 0x24;`, on the theory that target's `addiu $a3,$a3,0x24` is a
 * strength-reduced giv of i rather than a source-level offset variable.
 * RESULT: loop.c produces the identical giv either way; score, insn count and
 * the sum/bp/j seats are all unchanged.  `offset` vs `i * 0x24` is a free
 * spelling choice on this function and moves no RA input.
 */
s32 damage_DebugDisp(s32 *arg0) {
    u8 *base = (u8 *)arg0;
    s32 i;
    s32 *chkptr;

    i = 0;
    chkptr = (s32 *)base;
    do {
        s32 sum;
        u8 *bp;
        u32 j;

        sum = 0;
        bp = base + i * 0x24;
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
