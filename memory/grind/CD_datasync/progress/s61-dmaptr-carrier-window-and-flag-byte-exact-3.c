/* CD_datasync - SESSION 61 (rederive).  SCORE 3 / 91.
 *
 * WHAT THIS IS.  The s60 CD_ready transplant chassis, with chain A's address
 * staged through `dma` - the CD DMA-control POINTER, which gets its second,
 * REAL set in the function tail (`dma = D_800A14C0;` feeding the
 * `*dma & 0x1000000` test).  That is the first carrier in 61 sessions that is
 * multi-set, non-foldable, and NOT the -1/0 flag.
 *
 * RESULT.  The whole do_timeout window (asm/funcs/CD_datasync.s:45-63) AND the
 * flag block (68-72: `j ; li v0,-1 / move v0,zero / bnez v0 ; li v0,-1`) are
 * BYTE-EXACT, seats included.  s60's 3-insn flag residual is gone.  The entire
 * remaining residual is three instructions in the tail, where the carrier's
 * second live range takes $a0 and the target wants $v0:
 *      ours   lui a0 ; lw a0,%lo(D_800A14C0)(a0) ; lw v0,0(a0)
 *      target lui v0 ; lw v0,%lo(D_800A14C0)(v0) ; lw v0,0(v0)
 *
 * WHY IT CANNOT BE FIXED BY MOVING THE CARRIER (s61 measurements).  The order
 * needs sched.c:2526 birthing_insn_p to return 0 on chain A's `addu`, i.e.
 * reg_n_sets[chainA-addr] > 1 at flow time.  Every second set that cse can
 * fold is erased (puts-string reuse 8, format-string reuse 11, base-before-
 * the-wrap + index-inside 8/8/8), and every set that survives lives in a
 * DIFFERENT basic block, so the pseudo becomes a global-alloc quantity with
 * ONE hard register for both ranges.  The target's window seat is $a0 and
 * every tail value's seat is $v0/$v1, so any surviving carrier pays 3-4 in
 * its own block: v0-flag 3 (s60), cnt 4 (s60), DMA pointer 3 (this file),
 * DMA status word 3, masked flag 4.  Merging the tail pointer with chain B
 * instead of chain A loses the order outright (8).
 *
 * CONSEQUENCE FOR THE MODEL.  In the TARGET the chain-A address cannot be the
 * same pseudo as any tail value (different hard registers), so the original
 * source did not get its order from a spanning multi-set carrier.  Either its
 * chain-A pseudo had a second set that produced no bytes (a copy coalesced by
 * the allocator - `q = dma;` measured, coalesced, still 3), or the order comes
 * from local-alloc priorities rather than the sched boost.  That second option
 * is the untried one: on the single-set chassis (rejected/s61-fresh-single-set
 * -carrier-seat-swap-8.c) the emitted window is NOT mis-ordered so much as
 * SEAT-SWAPPED - chain A takes $v1 and the arg5 value takes $v0, exactly the
 * inverse of the target - which is a local-alloc qty_compare question the
 * ra_solver models directly.
 */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
    volatile u32 *dma;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &g_cd_status_a;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    puts(&g_str_cd_timeout);
    {
        s32 arg5;
        s32 t0;
        s32 *pB;
        s32 tb;
        void **pp;
        t0 = idx_1494[0];
        do {
            tb = idx_1494[1];
            pB = (s32 *)((tb << 2) + (s32)tbl_125c);
            dma = (volatile u32 *)((t0 << 2) + (s32)tbl_125c);
            arg5 = *pB;
            pp = &D_800F19C0;
            printf(&D_800161C8, *pp, tbl_11dc[D_800A11D5], *(s32 *)dma, arg5);
            CD_flush();
        } while (0);
    }
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    dma = D_800A14C0;
    if (*dma & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
