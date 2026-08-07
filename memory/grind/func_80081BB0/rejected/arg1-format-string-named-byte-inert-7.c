/* REJECTED — s12: the debug_printf format-string ADDRESS hoisted into a named local
(inside the wrapper, before the wrapper, and the same for the
tslTm2LoadImage_2 argument) = 7/91, byte-identical. All 51 prior forms only
ever touched args 2-5; arg1 is now measured and it is a dead axis.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = sys_VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
    tbl_125c = D_800A125C;
    D_800F19BC = 0;
    D_800F19C0 = &D_800162C0;

loop:
    v0 = sys_VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = D_800F19BC;
    D_800F19BC = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    do {
        s32 arg4;
        s32 *fmt;
        tslTm2LoadImage_2(&D_800161B8);
        fmt = &D_800161C8;
        arg4 = tbl_125c[idx_1494[0]];
        debug_printf(fmt, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     tbl_125c[idx_1494[1]]);
        cdrom_ClearIrq();
    } while (0);
    v0 = -1;
    goto check;

success:
    v0 = 0;

check:
    if (v0 != 0) {
        return -1;
    }
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
