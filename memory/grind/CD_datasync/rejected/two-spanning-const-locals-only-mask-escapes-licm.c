/* REJECTED (s3) — 27 at 97 insns (candidate is 18 at 92).
 *
 * WHAT IT PROVES: the REAL loop.c gate on hoisting a loop-invariant
 * constant is at tools/gcc-2.7.2/loop.c:695 — a movable is skipped only
 * when ALL THREE of these fail:
 *   (A) ! maybe_never && ! loop_reg_used_before_p (...)
 *   (B) ! REG_USERVAR_P (dest) && ! REG_LOOP_TEST_P (dest)
 *   (C) reg_in_basic_block_p (p, dest)
 * so a USER variable (B false) whose live range CROSSES a branch (C false)
 * and whose set sits where maybe_never is already 1 (A false) stays
 * materialised inline, in its own pseudo — with NO double-set required.
 *
 * MEASURED: `k2 = 0x1000000;` placed at the `check:` join so it spans the
 * `if (v0 != 0)` branch DOES escape the hoist (y3, mask-only, 27/97 vs the
 * 30/98 both-hoisted baseline).  `k1 = 0x3C0000;` placed at the top of the
 * loop body does NOT (y2, 30/98): maybe_never is still 0 that early — it
 * only becomes 1 at the first JUMP_INSN/CODE_LABEL inside the loop
 * (loop.c:930) — so branch (A) still holds and the movable is built.
 *
 * Both constants inline is therefore still only reachable via the
 * candidate's n_times_set==2 route (loop.c:702), which forces ONE pseudo
 * and hence ONE register.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k1;
    s32 k2;
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
        k1 = 0x3C0000;
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        if (!(k1 < cnt)) {
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
        k2 = 0x1000000;
        if (v0 != 0) {
            ret = -1;
            break;
        }
        if (!(*D_800A14C0 & k2)) {
            ret = 0;
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
