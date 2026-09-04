/* CD_datasync - THIRD LIVE CHASSIS, promoted from rejected/ by session s38.
 * Re-measured 2026-09-04 on the current chassis: 15 / 91, build_insns 91.
 *
 * WHY THIS FILE IS LIVE DESPITE SCORING 15 (candidate.c scores 7):
 * s38 typed all three chassis with tools/sched_solver (model validated on this
 * TU, parity=True, CD_datasync 12/12 blocks exact in both passes) and found the
 * two better-scoring chassis are foreclosed on OPPOSITE halves of target's
 * printf window, at depth 1 over EVERY modelled atom:
 *   * candidate.c  cannot defer arg4's value load  (--goal-before 100:131 and
 *     100:125 both return no vector over 1165 atoms);
 *   * the fully-inline chassis cannot lead the window with idx0's `lbu`
 *     (--goal-before 112:104 96:104 93:104, no vector over 1166 atoms).
 * THIS form leads the window with idx0's `lbu` (pass-2 emission
 * 97,108,93,120,100,102,111,113,123,104,115,125,127,135,129 - UID 97 is
 * idx0's lbu at slot 1, which is target's slot 1), and BOTH deferral facts
 * type REACHABLE here: `--goal-before 104:115` returns several vectors and
 * `--goal-before 104:135` returns exactly one, `del_dep 127 <- 104`.
 *
 * Its known residual (s33): the arg4 ADDRESS pseudo coalesces with the VALUE
 * pseudo into one register ($a3) where target keeps the address in $a0 and
 * loads the value into $a3 - a register-allocation question for tools/ra_solver,
 * not a scheduling one.
 *
 * NOTE: the rejected/ copy of this file is cp1252-encoded; this copy is UTF-8.
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
    /* FAKE: do{}while(0) — loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 i4;
        s32 arg5;
        s32 arg3;
        puts(&g_str_cd_timeout);
        i4 = idx_1494[0];
        arg5 = tbl_125c[idx_1494[1]];
        arg3 = tbl_11dc[D_800A11D5];
        printf(&D_800161C8, D_800F19C0, arg3, tbl_125c[i4],
               arg5);
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
