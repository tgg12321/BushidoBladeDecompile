/* REJECTED — s12: a per-access `((const s32 *)tbl_125c)[...]` cast on a FULLY INLINE arg4
= 13/91, byte-identical to the plain inline attractor (e8, with arg5 cast
too, is also 13). Combined with c1/e1 (the same cast on the NAMED-local arg4
= 9) this pins the mechanism: RTX_UNCHANGING_P only perturbs the schedule for
a load emitted at a STATEMENT; a load deferred to load_register_parameters is
already after the stack store and const changes nothing.
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
        tslTm2LoadImage_2(&D_800161B8);
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
                     ((const s32 *)tbl_125c)[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
