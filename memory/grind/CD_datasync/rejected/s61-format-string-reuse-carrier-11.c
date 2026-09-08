/* s61 REJECTED: score 11. Chain-A carrier reused as the printf format-string pointer. cse folds the constant set, so reg_n_sets stays 1 (order lost) AND the extra arg4 named intermediate displaces the value load.
 * Measured on the s60 CD_ready-transplant chassis (do{}while(0) wrap FAKE +
 * pp pointer-alias FAKE present); reference forms: v0-carrier 3, DMA-pointer 3. */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
    s32 *tbl_125c;
    char *pA;

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
        s32 arg4;
        s32 t0;
        s32 *pB;
        s32 tb;
        void **pp;
        t0 = idx_1494[0];
        do {
            tb = idx_1494[1];
            pB = (s32 *)((tb << 2) + (s32)tbl_125c);
            pA = (char *)((t0 << 2) + (s32)tbl_125c);
            arg5 = *pB;
            pp = &D_800F19C0;
            arg4 = *(s32 *)pA;
            pA = (char *)&D_800161C8;
            printf(pA, *pp, tbl_11dc[D_800A11D5], arg4, arg5);
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
    if (*D_800A14C0 & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
