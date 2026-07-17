/* REJECTED (s2b structural, 2026-07-17): the ENTIRE structural axis on the
 * 13-pair cross-jump wall is measured DEAD. Probes (harness
 * tmp/grind/motion_SetMotion/s2/s2b_probe.py, label-normalized cc1 asm diff
 * vs committed baseline m0 = 363 insns; merged shape = 361):
 *
 *   r1: ==3 arm re-associated set13-first
 *       (`sel = 0xD; if (v0 == 10) sel = 0xF; goto sel_dispatch;`)
 *       + honest 0xD at case 9/11 -> MERGED (361) AND arm branch sense
 *       flipped (bne + li13 in delay vs target's beq + li15): doubly wrong.
 *   r3: arm sense swap (`if (v0 == 10) sel = 0xF; else sel = 0xD;`)
 *       + honest 0xD -> identical to r1 (GCC canonicalizes): 122 diff lines.
 *   r2: sel narrowed to s16 + honest 0xD -> 364 insns, 658 diff lines.
 *   r4: block-local const split (`{ s32 d13 = 0xD; sel = d13; }`)
 *       -> const-prop folds to set13, MERGED (361).
 *
 * WHY the axis is closed (F1 measured this session, committed.i.jump2/.sched2
 * regenerated first-hand): the -1 pair's honest protection is sched2 SLACK —
 * case-10's block has 6 independent store insns, so sched2 hoists
 * `set s0<--1` (insn 374) up next to `s1<-0` (358), leaving the
 * `sb D_800A3350` (371) as the pre-jump insn; find_cross_jump iter1 then
 * mismatches (set-reg vs set-mem) -> 0 counted matches -> no merge in either
 * direction (label bonus needs >=1 match, jump.c:2532); reorg later pulls the
 * hoisted li back into the j's delay slot, so final bytes are the identical
 * [j; delay li -1] twins target has. The 13 sites CANNOT use this mechanism:
 * target fixes case-9/11 at exactly [jtbl-label; j; delay li 13] and the arm
 * at [beq; delay li15; j; delay li13], so in any byte-matching compile both
 * pre-jump insns are the identical `set s0<-13` with ZERO slack insns
 * available (arm: only the condjump-feeding compare upstream, uncrossable;
 * case-9/11: nothing). With iter1 forced to match and case-9/11's own jtbl
 * CODE_LABEL granting the stream-1 label bonus (--minimum -> 0), the merge is
 * GUARANTEED for every sanctioned spelling. The only byte-free RTL that
 * breaks iter1 is a USE/CLOBBER insn, whose sole C-reachable emitters
 * (union-constructor expr.c:2996, struct-return calls.c:1784) are the
 * manufacture family the Judge ruled unsanctioned (decisions.md 2026-07-17
 * 17:09 FAIL). NOTEs are skipped everywhere (wrap family measured dead,
 * s2 rejected file); labels unify at jump2 (m7/s1). Remaining paths are
 * policy-level, not structural. */
