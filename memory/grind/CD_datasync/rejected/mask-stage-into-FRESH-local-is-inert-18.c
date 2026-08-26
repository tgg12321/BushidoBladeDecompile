/* KILLED (s4, H16).  The 0x1000000 mask staged out of the double-set holder
 * `k` into a FRESH local (`s32 m; k = 0x1000000; m = k;`) measures 18 / 92 —
 * byte-identical to the session-3 candidate, i.e. completely inert.  The
 * identical stage into the ALREADY-LIVE loop counter `cnt` measures 11 / 93.
 * So the effect is not "an extra copy insn"; it is the reuse of a pseudo that
 * already has a live range, which is the [[defeat-licm-hoist-var-reuse]] /
 * [[staged-value-reused-variable]] signature.  Recorded so no later session
 * re-tries the clean-local spelling hoping it is a non-cheat equivalent.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k;
    s32 m;
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
            ret = -1;
            break;
        }
        k = 0x1000000;
        m = k;
        if (!(*D_800A14C0 & m)) {
            ret = 0;
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
