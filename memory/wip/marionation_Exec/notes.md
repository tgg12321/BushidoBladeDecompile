# marionation_Exec - CANDIDATE MASKED 4 (mh5 form; region-1 down to ONE insn-pair)

## SESSION-4c: the twin h5 mirror lands (candidate.c = mh5)
Adding `v0 <<= 2; arg5 = *(s32 *)(v0 + (s32)tbl_125c);` (the twin's h5
half-staging) collapses region-1 to the SAME residual as the twin: ONE
{sll4@56 <-> addu5@57} pair. Total masked 4 = that pair (2) + region-3
(the arm-2 beqz-a2 delay nop + the resulting 1-slot shifts). The two
functions now share an identical region-1 residual and mirrored
candidates. Twin floor = masked 2 (h5). Permuters grinding both bases.

## SESSION-4 (2026-07-03): the v0-idx discovery + the review outcome
A form reaching masked 4 (vs the 6 floor) was found and twice FAILed
layer-1 review: rejected/v0-idx-staging-scheduler-pass.c — the index
staged through the dead-here v0 var strips the load LAUNCH priority
(sched.c adjust_priority/birthing_insn_p, reg_n_sets>=2) and re-times
the head to target (51-54+58-67 byte-match; t0=a0 via the p109 a0-pref
chain, FINDREGDBG-verified). Reviewer: live store = neither dead-store
nor named-local carve-out fits; the scheduler-pass mechanism is outside
the SOTN-evidenced LICM/RA variable-reuse scope -> needs fresh SOTN
evidence or a USER POLICY RULING. OWNER SANCTIONED IT 2026-07-03 (rule:
staged-value-reused-variable). candidate.c = the masked-4 form again;
residual = [55-57 sll4-rotation, luid-pinned] + region-3 (nop + shifts).
The bare FAKEs
(idx_1495/D_800F19C0/pp) now carry full mechanism annotations per the
review. Lever-exhaustion ledger for the block (all sandbox-measured):
plain 6; REU 10; carriers i/status/cnt/temp 9-30; unions M1/M3 9-30;
18+ order sweeps 4-37; split-addu 8+drift; m2c-inline 14. Permuter
(tmp/perm_mar6, honest splice-metric): 200x2/220x3/225 finds = the
X-hoist family, sandbox-verified WORSE (weighted-vs-masked divergence);
grinding on.

## (session-3 door-map folded: see cpu_side_move_dir_4 notes + git history)

## STATE (READ FIRST)
candidate.c = the score-6 form (`bash tools/mar_test_candidate.sh` applies
+ scores + restores; copy to tmp/mar_candidate.c first). 178/179 insns;
ALL 8 callee-saved + block regs target. Remaining:
- **Region-1 (printf lbu5/sll4 order, 4 masked lines)** — ledger below.
- **Region-3 (check2 slot steal + arm-2 move/la transposition, 2 lines +
  missing nop)** — decomposed + locally EXHAUSTED below.

## Region-2 SOLVED — recipe (in candidate.c)
Per-arm dst/dst2 split + arm-1 dst-EARLY. make_regs_eqv:853 (last-uid
rule) keeps a1 canonical → beqz s4; dbr backward scan takes the move,
sb protected. check-1 NOP: sb trap+oppmem, la never splits, SEQ stops,
1723 refuses conditional-SEQ steals.

## Region-3 — MECHANISM CLOSED (session-4c re-measure on the mh5 foundation)
The ONLY difference vs target: cc1 dbr steals `move a1,s4` past the sb
into the beqz-a2 slot (everything after = a 1-slot shift; sxs probe
tmp/mar_region3_sxs.py). Both loops' C is u3-style guards; loop-1's
steal MATCHES target, only loop-2's must be refused.
**Shapes that refuse the steal exist** — s1/u1/u3/u5/u6 (dst2 init moved
inside `if (a1 != 0)`) give 179/179 WITH the nop: the second branch then
tests the s-reg directly and follows the sb immediately, so the fill scan
hits a jump with nothing eligible. **But all of them flip the s-regs**:
the guard's extra a1-var read raises p73 (a1-holder) refs 4->5, pri
930->1176 vs the lui-base pair's 933 (3-point margin, ALLOCDBG probe
tmp/mar_u3_alloc.py) -> allocates 1st instead of 4th -> s1<->s4 +
s2/s3 rotate (masked 22-26). Counter-levers CLOSED by arithmetic:
refs back to 4 breaks loop-1's bytes (its guard must keep the a1-test);
livelen needs +23 luids that don't exist past the block; embedded
assign `if ((dst2 = a1) != 0)` CSEs back to base RTL (v1: byte-identical
to base, steal intact); src/i movement alone flips WITHOUT the nop
(u2/u4/v2-v4). Old (3a)/(3b) text: git history. The permuter
(tmp/perm_mar6, honest splice metric, rebased on mh5) owns the search.

## Region-1 ledger (LARGELY SUPERSEDED by session-4's v0-idx fix; full text in git)
The old tie ([temp+t0]↔val5 birth-order) and the scheduler theorem
(rank_for_schedule → schedule_select hazard-greedy; LAUNCH-tied pairs
force ALU-before-load) remain the mechanism reference — see git history
and the twin's notes. The v0-idx staging resolved the head; only the
55-57 sll4-rotation survives (luid-pinned, see session-4 block).

## Target ground truth (asm/funcs/marionation_Exec.s)
- Regs: status s0, saved s1, i1494 s2, i1496 s3, arg1 s4, tbl s5,
  i1495 s6, arg0 s7; tail check a2, src a0, i v1, b v0, dst/dst2 a1.
- check-1: lbu 0(s3), andi a2, beqz→check2, NOP. arm-1: sb 0(s3), la
  F19B0, beqz s4→.L812BC, slot=move; li v1,7; li a3,-1; loop; j; move.
- check2: lbu -1(s3), andi, beqz→after_blocks, NOP. arm-2: sb -1(s3),
  MOVE, la F19A8, beqz a1→.L812BC, slot=li v1,7; li a3,-1; loop;
  .L812BC: j .L812CC; move v0,a2. after_blocks: beqz s7→loop, slot=
  move v0,zero. Two [j; move] sites unmerged (ours ✓).

## Known gotchas
- 42 rules index-anchored; end gate = retire-all-42 + full SHA1. Twin
  csmd4 (:388) shares text — marionation-unique anchors. Declare
  new_var/new_var3 in the final form (undeclared-at-:555; bytes fine).
  csmd4's last 5 rules are THIS printf block; mechanisms transfer.

## Tools
- tmp/gccdbg/cc1: BB2_DBR_DEBUG, BB2_NO_FT_STEAL, BB2_ALLOC_DEBUG,
  QTY/SCHED/SLL/FLOW. Runners: tmp/mar_{dbrdbg,allocdbg,qtydbg,
  cand_sched}.sh; mar_dbr_tail.py; mar_i2_trace.py.
- Sweeps: tmp/mar_qty_sweep{,2,3,4}.py, mar_family_sweep.py,
  mar_{cross_sweep,nv2_sweep,head_order,perm_leads}.py.
- Kit: mar_test_candidate.sh + mar_diff2.sh (restore src).