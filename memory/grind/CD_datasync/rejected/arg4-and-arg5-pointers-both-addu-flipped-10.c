/* REJECTED (s25). Both arg4 and arg5 as address pointers. lev 10; BOTH addu's flip
 * to base-first, doubling the operand-order divergence from target. Confirms the
 * flip is a property of the pointer-typed source expression, not of arg4. */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    u8 *idx_1494;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = D_800A11DC;
    idx_1494 = &D_800A1494;
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
    /* FAKE: do{}while(0) */
    do {
        s32 *p4;
        s32 *p5;
        puts(&g_str_cd_timeout);
        p4 = idx_1494[0] + tbl_125c;
        p5 = idx_1494[1] + tbl_125c;
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4,
               *p5);
        CD_flush();
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
