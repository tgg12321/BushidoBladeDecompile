/* REJECTED (grind session 6) - 14 / 91.
 *
 * Both table lookups written INLINE in the debug_printf call (no named
 * intermediate at all).  This is the SESSION-1 "single expression call" shape
 * re-measured on the correct cluster-A chassis: 14, not session 1's 23.
 *
 * WHY IT IS BANKED HERE AND NOT SIMPLY DISMISSED: forensically this is the
 * form whose EXPAND-time RTL matches target.  Read from the .rtl dump, an
 * inline call argument's final load is emitted by expand_call DIRECTLY into
 * the hard argument register as the LAST insn of the argument sequence:
 *   (insn 129 (set (reg:SI 7 a3) (mem/s:SI (reg:SI 99))))
 * and target's block likewise ends with `lw a3,0(a0)` as its last memory
 * reference.  The 8/91 candidate, which names arg4, gets
 *   (insn 101 (set (reg/v:SI 88) (mem/s:SI (reg:SI 93))))   <- at the stmt
 *   (insn 134 (set (reg:SI 7 a3) (reg/v:SI 88)))
 * and local_alloc then gives pseudo 88 $a3 by copy preference, so the LOAD
 * itself is written to $a3 early - which target does not do.
 *
 * So the 8 and the 14 are two different basins and the 8's fourth argument is
 * structurally WRONG even though it scores better.  The residual of this form
 * is that sched1 schedules the whole idx_1494[0] chain at the END of the block
 * (positions 9-15 of 16) where target spreads it from position 1 to the last
 * insn, which is also why target's address temp lands in $a0 and ours in $v0.
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
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
