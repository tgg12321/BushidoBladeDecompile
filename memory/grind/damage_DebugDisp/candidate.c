/* Candidate body for damage_DebugDisp — score 8 vs HEAD's 9.
 * Sole lever: for-loop fence on the 4-block CopyBlock loop
 * (loop-exit-work-inside-loop-sched-fence; user-sanctioned 2026-06-11).
 * Materialises the load-delay nop and places k=0 after the sw, matching
 * target's strictly source-ordered post-loop region. Retires the
 * `insert "nop" @ 53` and `reorder 55,54 @ 54-55` regfix rules.
 *
 * Remaining gap (8 diffs, all reg-alloc/scheduling — no clean lever found):
 *   1) Inner sum-loop $a0/$a1 swap (~3-4 diffs) — target $a0=sum/$a1=j,
 *      build $a0=j/$a1=sum.
 *   2) Second-loop preheader: target emits the 3 const-load insns
 *      (lui $t0,0x8000; lui $a3,0x1f; ori $a3,0xffff) BEFORE the 2 moves
 *      (ap=base; a2p=ap), build emits moves first (~5 diffs).
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
        u8 *bp;
        u32 j;

        j = 0;
        bp = base + offset;
        sum = 0;
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
        s32 k = 0;
        s32 *ap;
        u8 *a2p;

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
            k = 0;
        }

        ap = (s32 *)base;
        a2p = base;
        do {
            u16 *ptr = *(u16 **)((u8 *)ap + 0x78);
            if ((u32)((u32)ptr - 0x80000000U) <= 0x1FFFFF) {
                *ptr = *(u16 *)(a2p + 0xD0);
            }
            a2p += 2;
            k++;
            ap++;
        } while (k < 0x16);
    }

    return 1;
}
