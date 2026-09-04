/* REJECTED (s25). The arg4 pointer initialized BEFORE puts(), so it must survive
 * the call: GCC pins it in a fifth callee-saved register ($s0 = addu $s1,$s0 in the
 * prologue-adjacent code), the frame grows, and the whole allocation rotates.
 * lev 31, 84 build insns vs 91 target. Any spelling that makes a window value live
 * across the puts() call pays a callee-save. */
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
        s32 *p4 = &tbl_125c[idx_1494[0]];
        puts(&g_str_cd_timeout);
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *p4,
               tbl_125c[idx_1494[1]]);
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
