/* REJECTED — session 2.  Score 19 (candidate 18); 93 build insns vs 91.
 * Two spellings measured, both 19/93:
 *   v11 = `do { ... if (v0 != 0) return -1; ... if (!(mask)) return 0; }
 *          while (a0 == 0); return 1;`     (this file)
 *   v13 = `for (;;) { ... return -1; ... return 0; ... if (a0 != 0)
 *          return 1; }`
 *
 * IDEA: the candidate's remaining +2 is a TAIL BLOCK LAYOUT problem — our
 * mask exit comes out as
 *      bnez v0,<cont> / nop / j <end> / move v0,zero
 * where target has the un-inverted fall-through form
 *      beqz $v0,.L80081CFC / addu $v0,$zero,$zero        (in the delay slot)
 * Target's tail reads as three inline `return`s (each constant delivered in
 * the branch delay slot: `addiu $v0,-1`, `addu $v0,$zero,$zero`,
 * `addiu $v0,0x1`), so replacing the candidate's `ret = N; break;` pattern
 * with real `return N;` statements looked like the natural fix.
 *
 * RESULT: 19 / 93 — one WORSE than the candidate, and the inverted-branch +
 * `j end` pair is still there.  GCC still lays the `ret = 1 / back-edge test`
 * block AFTER the exit jump rather than before it; the inline returns just
 * add a third epilogue-bound path without changing the ordering decision.
 * (Session 1 measured the same relative ordering on the pre-reuse real-loop
 * form: shared-ret 29/98 beat inline-returns 30/99.  The shared-`ret` shape
 * is better in BOTH regimes; that is now measured twice.)
 *
 * VERDICT: the tail residual is NOT about return-vs-break spelling.  The next
 * probe has to attack the block ORDER (which side of the mask branch falls
 * through), e.g. by inverting the mask test's sense in the source, or by
 * moving the loop-continue condition into the same statement as the mask
 * test.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 k;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

    do {
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        k = 0x3C0000;
        if (!(k < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        {
            s32 arg5, arg4;
            arg5 = tbl_125c[idx_1494[1]];
            arg4 = tbl_125c[idx_1494[0]];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
        }
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            return -1;
        }
        k = 0x1000000;
        if (!(*D_800A14C0 & k)) {
            return 0;
        }
    } while (a0 == 0);
    return 1;
}
