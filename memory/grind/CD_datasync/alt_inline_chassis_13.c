/* CD_datasync — THE FULLY-INLINE ARGUMENT CHASSIS.  Sandbox score 13 / 91
 * (measured live, session 25, on the current HEAD chassis).
 *
 * THIS FILE IS NOT A REGRESSION BANK — IT IS THE STRUCTURALLY CORRECT CHASSIS.
 * Session 24 showed from the .combine RTL dumps that this form carries the
 * TARGET's pre-scheduling dependence graph (arg3 and arg4 loaded AT the
 * register-load point, arg5's value in a pseudo stored to 16($sp)); session 25
 * confirmed that independently with tools/sched_solver: of the fourteen
 * ordering constraints that define target's window, EIGHT are satisfied by
 * this form's baseline schedule versus SIX by candidate.c (which scores 7),
 * and the two constraints that are hardest on candidate.c — "arg3's lw $a2 is
 * emitted before arg4's lw $a3", which is UNREACHABLE there by any modelled
 * input change at all — are free here.
 *
 * The sandbox score is a MISLEADING gradient on this function (s23 recorded
 * the same lesson from a different direction).  Any future session that
 * chooses a chassis by score will choose candidate.c and will be working the
 * wrong graph.
 *
 * WHAT IS STILL WRONG HERE (s25, solver-typed): the joint window order is not
 * reached from this chassis by any single modelled input change (1332 atoms)
 * and by no exhaustive pair of SPELLABLE atoms (693 luid/luid_move atoms, i.e.
 * every ordinary source-statement move, 267 s).  The solver's preferred atom,
 * recurring across four separate constraints, is `add_dep 96 <- 107`: arg3's
 * `lbu D_800A11D5` must depend on arg4's `sll`.  Those two insns share no
 * register and no memory in this program, so that edge has no C spelling under
 * the ledger's current reading of the printf argument list.
 *
 * UID map for this chassis (pass 1, block 3), for re-running the solver:
 *   93 arg2 lw D_800F19C0 | 96 arg3 lbu D_800A11D5 | 99 arg3 sll | 101 arg3 addu
 *   104 idx0 lbu | 107 idx0 sll | 109 idx0 addu | 112 idx1 lbu | 115 idx1 sll
 *   117 idx1 addu | 119 arg5 lw | 121 sw 16($sp) | 123 la $4,fmt
 *   127 arg3 lw $a2 | 129 arg4 lw $a3 | 131 jal printf
 * Target emission order in those UIDs:
 *   104,112,93,115,117,107,119,96,109,99,101,121,127,129,123
 * Reproduce with tmp/grind/CD_datasync/s25/{blk2.py,solve_inl.py}.
 * ========================================================================= */
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
    /* FAKE: do{}while(0) â€” loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        puts(&g_str_cd_timeout);
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5],
               tbl_125c[idx_1494[0]], tbl_125c[idx_1494[1]]);
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
