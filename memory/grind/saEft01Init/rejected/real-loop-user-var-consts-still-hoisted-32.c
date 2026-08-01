/* REJECTED (s9, H37): on the real-loop `clean` chassis, putting BOTH compare
 * constants in named user locals at the top of the loop body does NOT defeat
 * loop.c's LICM. 32 / 96, byte-identical to `clean`. The .loop dump shows both
 * still built as movables (`Insn 43: regno 76 (life 12) ... moved to 203`,
 * `Insn 46: regno 77 (life 54) ... moved to 205`) because disjunct (1) of the
 * loop.c:691-701 gate is TRUE at the top of the loop body: maybe_never is 0
 * there and the reg is not used before its set. REG_USERVAR_P alone buys
 * nothing.
 */
/* q2: both constants as user locals set at the loop-body top */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 lim;
    s32 msk;
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
        lim = 0x3C0000;
        msk = 0x1000000;
        v0 = sys_VSync(-1);
        if (D_800F19B8 < v0) {
            goto do_timeout;
        }
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        if (!(lim < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
                     tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
        cdrom_ClearIrq();
        v0 = -1;
        goto check;

    success:
        v0 = 0;

    check:
        if (v0 != 0) {
            ret = -1;
            break;
        }
        ret = *D_800A14C0 & msk;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
