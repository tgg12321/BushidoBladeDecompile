/* CD_datasync — SESSION 45 (synthesis).  Banked ALTERNATE 7 / 91 form.
 * sandbox CD_datasync --disable all => score 7, target_insns 91, build_insns 91
 * (confirmed with the real engine this session, not only on the s21 harness).
 *
 * WHY THIS FILE EXISTS ALONGSIDE candidate.c (which also scores 7)
 * ================================================================
 * It is BYTE-DISTINCT from candidate.c and it is the first form in 45 sessions
 * that emits arg2's `lui $a1,%hi(D_800F19C0) / lw $a1,%lo(...)` pair at
 * TARGET'S OWN slots 43/44.  candidate.c (the m011 chassis) emits that pair at
 * 51/52 and has never been made to move it: s42/s43/s44 measured every arg2
 * spelling (named value, pointer alias, hoisted, inline, assigned first/last)
 * as byte-inert on that chassis, and this session re-confirmed that with 30
 * more forms.  The lever that moves the pair is NOT an arg2 spelling at all —
 * it is the ORDER OF THE TWO tbl_125c VALUE STATEMENTS:
 *
 *   value order arg4-then-arg5 ("45")  -> leaf lbu order is idx0,idx1 (TARGET)
 *                                          arg2 pair at 51/52   (WRONG)
 *   value order arg5-then-arg4 ("54")  -> leaf lbu order is idx1,idx0 (WRONG)
 *                                          arg2 pair at 43/44   (TARGET)
 *
 * Both halves score 7; the two correct features are, so far, mutually
 * exclusive.  This file is the "54" representative, kept so a later session can
 * attack the fork from the arg2-correct side instead of always from the
 * leaf-correct side (candidate.c).
 *
 * Window alignment measured this session (build | target, build idx 41-56):
 *   41 !! lbu $v1,1($s1)      | lbu  $a0,0($s1)
 *   42 !! lbu $v0,0($s1)      | lbu  $v0,1($s1)
 *   43    lui $a1,%hi(S)      | lui  $a1,%hi(S)      <-- MATCH (new)
 *   44    lw  $a1,%lo(S)($a1) | lw   $a1,%lo(S)($a1) <-- MATCH (new)
 *   45 !! sll $v1,$v1,2       | sll  $v0,$v0,2
 *   46 !! sll $v0,$v0,2       | addu $v0,$v0,$s0
 *   47 !! addu $v0,$v0,$s0    | sll  $a0,$a0,2
 *   48 !! addu $v1,$s0,$v1    | lw   $v1,0($v0)
 *   49 !! lw  $a3,0($v0)      | lui  $v0,%hi(S)
 *   50 !! lui $v0,%hi(S)      | lbu  $v0,%lo(S)($v0)
 *   51 !! lbu $v0,%lo(S)($v0) | addu $a0,$a0,$s0
 *   52 !! lw  $v1,0($v1)      | sll  $v0,$v0,2
 *   53 !! sll $v0,$v0,2       | addu $v0,$v0,$s3
 *   54 !! addu $v0,$v0,$s3    | sw   $v1,16($sp)
 *   55 !! sw  $v1,16($sp)     | lw   $a2,0($v0)
 *   56 !! lw  $a2,0($v0)      | lw   $a3,0($a0)
 * The instruction MULTISET is identical to target's here (modulo the $v1/$a0
 * naming of the idx0 chain, which s10 proved is downstream of the order); the
 * whole residual on both 7-forms is a sched2 emission ORDER fact.
 *
 * The do{}while(0) wrapper is unchanged from candidate.c and carries the same
 * sanctioned FAKE annotation (owner ruling 2026-07-06, do-while-zero-exception).
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
        s32 k5;
        s32 val4;
        puts(&g_str_cd_timeout);
        k5 = idx_1494[1] * 4;
        val4 = tbl_125c[idx_1494[0]];
        printf(&D_800161C8, D_800F19C0, tbl_11dc[D_800A11D5], val4,
               *(s32 *)((u8 *)tbl_125c + k5));
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
