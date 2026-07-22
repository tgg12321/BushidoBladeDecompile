/* Candidate body for damage_DebugDisp — score 6 vs prior floor 8 (HEAD 9).
 * Levers:
 *   1) for-loop fence on the 4-block CopyBlock loop
 *      (loop-exit-work-inside-loop-sched-fence; user-sanctioned 2026-06-11).
 *   2) `do { sum = 0; } while (0);` — do-while(0) RA-weighting wrapper on the
 *      inner accumulator init (sanctioned for ANY codegen effect per the FINAL
 *      2026-07-06 do-while-zero-exception ruling). Found by the s4 directed
 *      permuter (full-TU basin, output-130-1). RESOLVES Region A: the inner
 *      loop sum/j $a0<->$a1 register swap is gone.
 *      MECHANISM (s7 correction; s6's "live_length" story was wrong — measured
 *      live_length=9 for sum(77) and j(79) on BOTH chassis): the do-while(0)'s
 *      NOTE_INSN_LOOP_BEG raises sum=0's block loop_depth 1->2, so its
 *      loop-depth-weighted reg_n_refs (`reg_n_refs += loop_depth`, flow.c) goes
 *      10 -> 11 = a TIE with j(11). global.c allocno_compare then breaks the tie
 *      by allocno number (sum pseudo 77 < j 79) -> sum takes $a0 (== target).
 *      Fragile: an exact priority tie decided by sum's lower pseudo number.
 *
 * Remaining gap (6 objdump diffs):
 *   A') Inner-loop preheader (2 diffs): target orders `addu $v1,$t1,$a3`
 *       (bp=base+offset) BEFORE `move $a1,$zero` (sum=0); build emits them
 *       swapped. New 2-insn scheduling tie introduced by the do-while(0).
 *   B)  Second-loop preheader (4 diffs): target emits the 3 const-load insns
 *       (lui $t0,0x8000; lui $a3,0x1f; ori $a3,0xffff) BEFORE the 2 moves
 *       (ap=base; a2p=ap); build emits moves first (LICM-hoist LUID tie).
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
        do { sum = 0; } while (0);
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
