/* saEft01Init — best form as of grind session 1 (recon).
 *
 * Honest pure-C distance (sandbox --disable all): 18   [floor unchanged from
 * the session-1 baseline, which was also 18]
 * Instruction count: 91 build vs 91 target (no ins/del — this is purely a
 * register-allocation + scheduling residual).
 *
 * WHAT CHANGED vs the inherited baseline:
 *   The debug_printf argument staging was reordered so the idx_1494[0] chain
 *   is evaluated BEFORE the idx_1494[1] chain (baseline staged arg5 = [1]
 *   first, then arg4 = [0]).  Score is identical (18), but this positionally
 *   fixes the two leading `lbu` in the arg block: target loads 0x0($s1) then
 *   0x1($s1); the baseline build loaded 0x1 then 0x0.  Structurally closer at
 *   equal score, so it is the better starting point.
 *   Side effect measured: with the swap, the D_800F19C0 (`$a1`) load moves
 *   LATE (build idx 56/57) whereas target has it early (idx 48/49), and the
 *   `lw $a3` for arg4 moves EARLY (build 52) whereas target has it last (61).
 *   The baseline ordering had the a1 load in the right place instead.  Both
 *   spellings score 18; the residual in this block is a scheduling trade,
 *   not a missing statement.
 *
 * The remaining 18 splits into two clusters (see evidence.md):
 *   A) a 3-way callee-save rotation: build puts the PARAM in $s0 and pushes
 *      tbl_125c/idx_1494 up to $s1/$s2; target wants $s0=tbl_125c,
 *      $s1=idx_1494, $s2=param, $s3=tbl_11dc.  MECHANISM IS SOLVED (loop
 *      notes — see hypotheses.md H3); the spelling is not.
 *   B) the debug_printf argument-block schedule described above.
 *
 * Do NOT re-try: inlining the three table globals at their use sites (kills
 * the hoisted base pointers entirely: 85 insns, score 40) or collapsing the
 * argument staging into one call expression (score 23).  Both are banked in
 * rejected/.
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
    tslTm2LoadImage_2(&D_800161B8);
    {
        s32 arg4, arg5;
        arg4 = tbl_125c[idx_1494[0]];
        arg5 = tbl_125c[idx_1494[1]];
        debug_printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], arg4, arg5);
    }
    cdrom_ClearIrq();
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
