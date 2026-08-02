/* saEft01Init — REJECTED (session 15, forensics).  sandbox 9 / 91.
 *
 * WHY IT IS BANKED: it is not a new attractor, it is the PROOF that the
 * 9-scoring "const chassis" (session 12's fourth attractor) is produced by
 * ONE dependence edge and nothing else.
 *
 * The only change from candidate.c is `((const s32 *)tbl_125c)[idx_1494[0]]`
 * on arg4's access — arg5 keeps the plain, aliasable spelling.  The full
 * `const s32 *tbl_125c` declaration (which marks BOTH accesses unchanging)
 * and this per-access cast on arg4 alone compile to the SAME sched1 order,
 * the SAME sched2 order and the SAME score.  So RTX_UNCHANGING_P on arg5's
 * load is completely inert, and the entire const effect is:
 *
 *   sched.c sched_analyze_1 stops emitting the REG_DEP_ANTI edge from insn
 *   100 (`arg4 = tbl_125c[idx_1494[0]]`, the `lw a3` value) to insn 125
 *   (`sw 16(sp)`, the outgoing stack argument).  Measured directly in the
 *   -da .sched LOG_LINKS:
 *     candidate  insn 125 ... (insn_list 89 (insn_list:REG_DEP_ANTI 100
 *                                            (insn_list 123 (nil))))
 *     this form  insn 125 ... (insn_list 89 (insn_list 123 (nil)))
 *   and the load's MEM goes `mem/s:SI` -> `mem/s/u:SI`.
 *
 * Removing that ONE edge is exactly what buys target's `sw 16(sp)` before
 * `lw a3` relation — and it costs 2, because sched1 then schedules the
 * idx[1] chain (116/119/121/123) ahead of the idx[0] `lbu` (93), so the two
 * `lbu` come out in the wrong order at the block head.  7 -> 9.
 *
 * Do NOT re-propose this, the whole-pointer const, or any other
 * RTX_UNCHANGING_P spelling: the axis is measured to the edge.
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
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of. */
    do {
        s32 arg4;
        tslTm2LoadImage_2(&D_800161B8);
        arg4 = ((const s32 *)tbl_125c)[idx_1494[0]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4,
                     tbl_125c[idx_1494[1]]);
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
