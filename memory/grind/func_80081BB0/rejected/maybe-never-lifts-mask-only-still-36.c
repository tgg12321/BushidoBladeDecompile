/* REJECTED (s9, H37): both constants in named locals set AFTER the loop's first
 * conditional branch (where maybe_never == 1). The MASK escapes the hoist for
 * the first time on this chassis (the .loop dump lists only `regno 76` as
 * moved), which confirms the gate reading exactly — but escaping is not enough:
 * the local's live range then spans the whole loop (4 refs / 98 insns, crosses
 * 5 calls) so it takes a callee-save anyway, where target has a block-local
 * `lui $v1,0x100` right before the `and`. 36 / 95. Block-local + not-hoisted is
 * unreachable in a real loop: block-local makes disjunct (3) true, which builds
 * the movable, which the threshold test then always moves.
 */
/* q5: both sets after the first in-loop branch (maybe_never == 1) */
s32 saEft01Init(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 ret;
    s32 lim;
    s32 msk;
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
        lim = 0x3C0000;
        msk = 0x1000000;
        cnt = D_800F19BC;
        D_800F19BC = cnt + 1;
        if (!(lim < cnt)) {
            goto success;
        }

    do_timeout:
        tslTm2LoadImage_2(&D_800161B8);
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
                     tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
        ret = *D_800A14C0 & msk;
        if (ret == 0) {
            break;
        }
        ret = 1;
    } while (a0 == 0);
    return ret;
}
