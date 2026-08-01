/* REJECTED (grind session 6) - 14 / 91, BYTE-IDENTICAL to n0 above.
 *
 * This is the session-5 frontier's first next-probe: split the fourth
 * argument's chain across two statements (`i0 = idx_1494[0];` as its own
 * statement before the call, the table lookup left inline) so the byte load
 * is emitted early and the `lw` late.
 *
 * KILLED WITH THE DUMPS.  The expand-time LUID order really does change - here
 * the idx_1494[0] lbu is insn 94, the FIRST insn of the block, where in the
 * both-inline form it is insn 104, the third chain, which INVERTS its LUID
 * relation to the D_800A11D5 lbu (94 vs 102 here, 104 vs 96 there).  The
 * pre-reload scheduler's output (.sched) is nevertheless the SAME order in
 * both, with that lbu at position 9 of 16, and the emitted .s bodies are
 * byte-identical (md5 5e95422b).
 *
 * Therefore the INSN_LUID tie-break at sched.c:2452-2455 is NOT what orders
 * this block; INSN_PRIORITY and the dependence-class test at sched.c:2412-2449
 * are, and both are functions of the dependency DAG alone.  No source-level
 * statement placement inside this basic block can move the argument block.
 * Variants n3 (both indices named) and n5 (arg5 named + i0 named) are the same
 * bytes again.
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
            s32 i0;
            i0 = idx_1494[0];
            debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], tbl_125c[i0], tbl_125c[idx_1494[1]]);
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
