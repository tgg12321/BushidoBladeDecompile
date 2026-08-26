/* REJECTED (s3) — 22 at 94 insns (candidate is 18 at 92).
 *
 * Motivated by target's register assignment: target holds 0x3C0000 in $v0
 * (idx 41, immediately consumed by the following `slt $v0,$v0,$v1`) and
 * 0x1000000 in $v1 (idx 83) — TWO registers, so the candidate's single
 * reused `k` cannot be the original spelling.  $v0 is also target's
 * timeout/success flag, so this variant makes the flag local `v0` carry
 * 0x3C0000 (it already has >1 set, so loop.c:702 blocks its hoist) and
 * gives the mask its own branch-spanning local `k2` (blocked by
 * loop.c:695).
 *
 * RESULT: both constants do stay inline, but overloading `v0` costs +2
 * insns in the flag block — the same regression session 2 measured for
 * v0-carrying-BOTH constants (H8, 22/93).  Confirms the flag local is not
 * usable as a constant holder in any combination.
 * Isolation control: v0-timeout + LITERAL mask is 29/95.
 */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 k2;
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
        v0 = 0x3C0000;
        if (!(v0 < cnt)) {
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
        k2 = 0x1000000;
        if (v0 != 0) {
            ret = -1;
            break;
        }
        if (!(*D_800A14C0 & k2)) {
            ret = 0;
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
