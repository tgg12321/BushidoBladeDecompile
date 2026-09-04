/* REJECTED: scaled-index-offset arg4 (k0 = idx[0]*4; *(s32*)((u8*)tbl+k0)) reaches TARGET's tail relation (sw 16(sp) before lw a3, lw a3 LAST, D5 chain in target's slots) and spreads the idx[0] chain like target does - but the arg5 lw/sw pair stays adjacent, costing a load-delay nop: 8 / 92. The whole scaled family (k0 only, k0+k1, shift vs multiply, reversed addu operands, arg3 also scaled, arg5 named/addr/offset) is ONE rigid attractor.
 * measured session 21 (2026-09-04, rederive), chassis floor 7/91.
 */
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
    do {
        s32 k0;
        puts(&g_str_cd_timeout);
        k0 = idx_1494[0] * 4;
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
               *(s32 *)((u8 *)tbl_125c + k0), tbl_125c[idx_1494[1]]);
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
