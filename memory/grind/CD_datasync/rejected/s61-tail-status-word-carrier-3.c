/* s61: score 3 (ties the DMA-pointer form, does not beat it). Chain-A carrier
 * merged with the tail's DMA STATUS WORD (st = *D_800A14C0; if (st & ...)).
 * The pointer load is then byte-exact but combine rewrites the carrier's tail
 * set into the AND, so the word lands in $v1, the mask in $v0 and the AND dest
 * in $a0 - three insns, the same price as the pointer spelling. Measured on the
 * s60 CD_ready-transplant chassis (wrap FAKE + pp alias FAKE). */
s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    volatile u8 *idx_1494;
    s32 *tbl_125c;
    s32 st;

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
            st = (s32)((t0 << 2) + (s32)tbl_125c);
            arg5 = *pB;
            pp = &D_800F19C0;
            printf(&D_800161C8, *pp, tbl_11dc[D_800A11D5], *(s32 *)st, arg5);
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
    st = *D_800A14C0;
    if (st & 0x1000000) {
        if (a0 == 0) {
            goto loop;
        }
        return 1;
    }
    return 0;
}
