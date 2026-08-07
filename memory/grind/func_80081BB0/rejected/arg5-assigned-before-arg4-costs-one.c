/* REJECTED (session 5, H20) — 9 / 91.
 *
 * The 2x2 cross-product of the two named argument intermediates' declaration
 * and statement order, measured on the CORRECT callee-save allocation for
 * the first time (session 1 measured the pair as tied at 18 under the wrong
 * allocation; that tie was an artefact — session-2 F6).  Result: 9 / 8 / 8 /
 * 9.  The statement order is the whole lever — assigning the idx[0] lookup
 * FIRST is worth one point — and the declaration order is byte-inert
 * (decl arg5,arg4 == decl arg4,arg5 at both statement orders).  This file is
 * the arg5-first spelling, kept as the measured negative.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
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
            ret = -1;
            break;
        }
        k = 0x1000000;
        cnt = k;
        ret = *D_800A14C0 & cnt;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
