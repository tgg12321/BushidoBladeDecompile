s32 CD_datasync(s32 a0) {
    s32 v0;
    s32 cnt;
    s32 *tbl_11dc;
    s32 *tbl_125c;

    D_800F19B8 = VSync(-1) + 0x3C0;
    tbl_11dc = CD_comstr; /* FAKE: pointer alias (second handle) to the libcd command-name table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s3 across the poll loop as the target does (asm/funcs/CD_datasync.s:11-12); lever-exhaustion: s62 ablation A4 (direct D_800A11DC[] subscript) = 18/88, plus the 61-session ledger in memory/grind/CD_datasync/hypotheses.md */
    tbl_125c = CD_intstr; /* FAKE: pointer alias (second handle) to the CD_intstr table per pointer-alias-fake-exception (owner ruling 2026-07-01), mechanism: global.c seats the base in $s0 across the poll loop as the target does (asm/funcs/CD_datasync.s:15-16); lever-exhaustion: s62 ablation A3 (direct D_800A125C[] subscript) = 27/89 */
    Alarm_plus_0x4 = 0;
    Alarm_plus_0x8 = &D_800162C0;

loop:
    v0 = VSync(-1);
    if (D_800F19B8 < v0) {
        goto do_timeout;
    }
    cnt = Alarm_plus_0x4;
    Alarm_plus_0x4 = cnt + 1;
    if (!(0x3C0000 < cnt)) {
        goto success;
    }

do_timeout:
    puts(&g_str_cd_timeout);
    {
        s32 t0;
        s32 tb;
        void **pp;
        t0 = Intr.sync; /* FAKE: named intermediate for the sync byte, placed BEFORE the wrap (loop depth 1), mechanism: flow.c loop_depth-weighted reg_n_refs feeds local-alloc.c qty_compare - the depth-1 mention leaves the merged chain-A quantity below the second table read's priority, so the chain takes $a0 and the value $v1 exactly as at asm/funcs/CD_datasync.s:50/56/60/65; lever-exhaustion: s62 probe B3 (same read placed inside the wrap) = 4/91, s62 probe B1 (inlined into the call) = 12/91, s60 in-place spelling = 15 */
        do { /* FAKE: do-while(0) wrap per do-while-zero-exception (owner ruling 2026-07-06), mechanism: sched.c:2081 loop-note barrier on the first insn inside (the ready-byte load) orders the sync-byte load ahead of it and every later register-argument load after it, and flow.c loop_depth ref weighting seats tbl_125c in $s0; lever-exhaustion: s62 ablation A2 (wrap removed) = 13/91, s61 ablation 2 -> 12 */
            tb = Intr.ready; /* FAKE: named intermediate for the ready byte (fresh, once-written, once-read, real value = lbu $v0,1($s1) at asm/funcs/CD_datasync.s:51), mechanism: expand argument staging keeps the fifth (stack) argument's chain out of the call sequence so the sw lands at slot 63; lever-exhaustion: s62 probe B1 (tb inlined into the call) = 12/91, s62 probe A6 (all intermediates inlined) = 13/91 */
            pp = &Alarm_plus_0x8; /* FAKE: pointer alias (second handle) to the alarm callback slot per pointer-alias-fake-exception (owner ruling 2026-07-01, the `Type* t = &g_Thing;` redundant-second-handle shape), mechanism: calls.c:1652-1664 expand_call precomputes a register argument whose rtx_cost > 2 into a pseudo inside a loop (preserve_subexpressions_p), whereas `*pp` is a cheap mem(reg) that stays in the call sequence and cse folds the alias back to the target's `lui $a1 / lw $a1` at asm/funcs/CD_datasync.s:52-53; lever-exhaustion: s62 ablation A1 (direct D_800F19C0 read) = 4/91 */
            printf(&D_800161C8, *pp, tbl_11dc[CD_com], tbl_125c[t0], tbl_125c[tb]);
            CD_flush();
        } while (0);
    }
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
