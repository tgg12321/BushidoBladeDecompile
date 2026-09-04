/* CD_datasync — REJECTED (s48).  engine 8 / build_insns 92 (one extra
 * load-delay #nop vs candidate).  Harness readout n=82 lev=7 nop=4.
 *
 * WHY IT IS DEAD, measured on the block-3 QTYDBG table, not on the score:
 * this is the representative of a 32-form lattice (arg4 in {inl,k,ik} x
 * arg5 in {v,iv,pv,pd} x every statement interleaving) built to execute the
 * s47 frontier item "arg5 spellings that complete the arg5 VALUE early
 * supply the missing $v1 occupancy".  28 of the 32 forms — every one with a
 * NAMED arg4 statement — produce a BYTE-IDENTICAL block-3 local-alloc
 * quantity table:
 *     q0 [8,14) refs12 -> $v0   (arg5 index+address chain)
 *     q1 [14,18) refs4  -> $v0  (arg5 VALUE)
 *     q2 [20,36) refs12 -> $v1  (arg4 address chain — the goal quantity)
 *     q3 [22,34) refs12 -> $v0  (arg3 address chain)
 * The arg5 VALUE quantity dies at luid 18, i.e. four luids BEFORE the arg3
 * chain is born at 22, in every spelling — named value, index+value,
 * pointer+value, pointer+inline-deref, at every statement position.  So no
 * arg5 spelling can make the arg5 value span the arg3 chain (feature F4),
 * and the $a0 seat for q2 is never approached.
 *
 * ROOT CAUSE, read out of the dumps (tmp/grind/CD_datasync/dumps): at
 * expand the stack-argument store (insn 128, `sw val5,16($sp)`) sits LATE,
 * between the arg4 address addu (126) and the argument-register moves
 * (130-136) — exactly target's slot-54 position.  The FIRST scheduling pass
 * (sched.c schedule_block, which runs BEFORE local_alloc) hoists it to
 * immediately after its only producer: post-.sched order is
 *   89 93 97 99 101 130 [128] 105 119 109 122 124 126 116 134 136 132 138
 * i.e. the sw lands directly behind insn 101 (`lw val5`).  That collapses
 * the arg5 value live range to 2 luids before local_alloc ever sees it.
 * F4 is therefore a sched1 placement fact, not an arg5-spelling fact.
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
    /* FAKE: do{}while(0) - loop_depth weighting for the three table pointers
     * without giving loop.c a loop to hoist the compare constants out of.
     * mechanism: flow.c loop_depth ref-weighting -> global.c allocno_compare.
     * lever-exhaustion: memory/grind/CD_datasync/hypotheses.md (s9 H37/H38). */
    do {
        s32 val5;
        s32 i4;
        s32 k4;
        puts(&g_str_cd_timeout);
        val5 = tbl_125c[idx_1494[1]];
        i4 = idx_1494[0];
        k4 = i4 * 4;
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], *(s32 *)((u8 *)tbl_125c + k4), val5);
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
