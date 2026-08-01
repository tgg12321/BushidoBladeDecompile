/* REJECTED (session 5, H19) — the session-4 distance-7 form.
 *
 * sandbox --disable all = 7 at 91 insns, i.e. it SCORES better than the
 * banked candidate, and it is nonetheless not a matching form: the third
 * lever re-bases the table pointer onto the first index and then indexes the
 * RE-BASED pointer with the second one, so `arg5` is `tbl[i0 + i1]` where
 * target passes `tbl[i1]`.  It is a semantic change to the value handed to
 * debug_printf, and it emits `addu s0,s0,v1` — an instruction target does
 * not contain.  Target computes both index chains off the unmodified base
 * register:
 *
 *   lbu a0,0(s1) / lbu v0,1(s1) / sll v0,v0,2 / addu v0,v0,s0 /
 *   sll a0,a0,2 / lw v1,0(v0) / addu a0,a0,s0 / sw v1,16(sp) / lw a3,0(a0)
 *
 * Session 4 checked only that the mutated pointer cannot be observed on a
 * LATER iteration; it never checked the value of arg5 on the current one.
 * Do not restore this form to chase the 7.
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
            tbl_125c = &tbl_125c[idx_1494[0]];
            arg5 = tbl_125c[idx_1494[1]];
            arg4 = *tbl_125c;
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
